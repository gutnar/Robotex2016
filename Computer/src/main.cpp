#include <opencv2/highgui.hpp>
#include <SimpleIni.h>
#include <sys/time.h>
#include <omp.h>

#include <getopt.h>
#include "capture.h"
#include "vision.h"

#include <iterator>
#include <CL/cl.hpp>
#include <CL/opencl.h>

#include "Communicator.h"
#include "Calibrator.h"
#include "Detector.h"
#include "AI.h"
#include "Blob.h"

using namespace cv;
using namespace std;
using namespace cl;


int main() {
    /// LOAD CONFIGURATION
    CSimpleIniA configuration;
    configuration.SetUnicode();
    configuration.LoadFile("configuration.ini");

    bool testingMode = configuration.GetBoolValue("settings", "TESTING");

    /// CREATE SRF COMMUNICATOR
    Communicator srf(configuration.GetValue("settings", "FIELD_ID")[0], configuration.GetValue("settings", "ROBOT_ID")[0]);

    try {
        /*
        int vendorId = atoi(configuration.GetValue("srf", "VENDOR_ID", "0"));
        int productId = atoi(configuration.GetValue("srf", "PRODUCT_ID", "0"));
        srf.connect(vendorId, productId);
         */
        srf.connect("/dev/ttyACM2");
    } catch (int exception) {
        cout << "Could not create serial RF connection!" << endl;
        //return 0;
    }

    /// CREATE SERIAL MOTHERBOARD COMMUNICATOR
    Communicator communicator;

    try {
        /*
        int vendorId = atoi(configuration.GetValue("motherboard", "VENDOR_ID", "0"));
        int productId = atoi(configuration.GetValue("motherboard", "PRODUCT_ID", "0"));
        communicator.connect(vendorId, productId);
         */
        communicator.connect("/dev/ttyACM0");
    } catch (int exception) {
        cout << "Could not create serial connection to motherboard!" << endl;
        //return 0;
    }

    // Start charging and get initial ball detector status
    communicator.send("c1\ni");

    /// START VIDEO CAPTURE
    VideoCapture cap;
    cap.open(0);

    /// CALIBRATION
    const string place = configuration.GetValue("settings", "PLACE", "");
    cout << ("calibration/" + place + ".ini") << endl;

    // Load calibration
    CSimpleIniA colorsIni;
    colorsIni.SetUnicode();
    SI_Error rc = colorsIni.LoadFile(("calibration/" + place + ".ini").c_str());

    // Calibrate
    if (rc < 0) {
        Calibrator calibrator;
        calibrator.calibrate(cap, &colorsIni);

        colorsIni.SaveFile(("calibration/" + place + ".ini").c_str());
    }

    /// SET UP DETECTOR
    // Detection detector(configuration file, colors file)
    Detector detector(configuration, colorsIni);
    //namedWindow("BALLS");

    /// SET UP AI
    AI ai(detector);

    /*
    for (int d = 0; d < 360; d += 15) {
        cout << d << " " << ai.getSpeedCommand(100, (float) d/180*3.1415926) << endl;
    }

    return 0;
     */

    // GAME STATUS
    bool gameIsOn = configuration.GetBoolValue("settings", "AUTOSTART");

    // TIMESTAMP
    struct timeval tp;
    gettimeofday(&tp, NULL);
    long int startTime = tp.tv_sec * 1000 + tp.tv_usec / 1000;

    // Load color data
    int colorMeans[NUMBER_OF_COLORS*3];
    int colorRanges[NUMBER_OF_COLORS*3*2];
    string keys[6] = {"H_MIN", "H_MAX", "S_MIN", "S_MAX", "V_MIN", "V_MAX"};

    for (int i = 0; i < NUMBER_OF_COLORS; ++i) {
        for (int j = 0; j < 6; ++j) {
            colorRanges[i*6+j] = (int) colorsIni.GetLongValue(COLORS[i].c_str(), keys[j].c_str());

            if (j % 2 == 0) {
                colorMeans[i * 3 + j / 2] = (atoi(colorsIni.GetValue(COLORS[i].c_str(), keys[j].c_str())) +
                                             atoi(colorsIni.GetValue(COLORS[i].c_str(), keys[j + 1].c_str()))) / 2;
            }
        }
    }

    /// GOAL COLORS
    int ownGoalColor = (int) configuration.GetLongValue("settings", "OWN_GOAL_COLOR");
    int opponentGoalColor = (int) configuration.GetLongValue("settings", "OPPONENT_GOAL_COLOR");

    /// KERNEL
    vector<Platform> platforms;
    vector<Device> devices;
    vector<Kernel> kernels;

    // create platform, context and command queue
    Platform::get(&platforms);
    platforms[0].getDevices(CL_DEVICE_TYPE_GPU, &devices);
    Context context(devices);

    // load opencl source
    std::ifstream cl_file("src/kernels.cl");
    std::string cl_string(std::istreambuf_iterator<char>(cl_file),
                          (std::istreambuf_iterator<char>()));
    Program::Sources source(1, std::make_pair(cl_string.c_str(),
                                              cl_string.length() + 1));

    // create program and kernel and set kernel arguments
    Program program(context, source);

    if(program.build(devices) != CL_SUCCESS){
        cout << "Build failed" << endl;
    }

    /// TEST IMAGE
    //Mat testImage;
    //testImage = imread("images/test3.png", CV_LOAD_IMAGE_COLOR);

    /// MAIN LOOP
    while (true) {
        /// IMAGE MANIPULATION
        Mat image;
        Mat workedImage;
        cap >> image;

        //image = testImage.clone();

        //namedWindow("cam");
        //imshow("cam", image);

        // Convert BGR to HSV
        cvtColor(image, workedImage, COLOR_BGR2HSV);

        /// IMAGE DATA FOR OPENCL
        //std::vector<uint> array(workedImage.rows*workedImage.cols);
        //array = workedImage.data;
        //workedImage.col(0).copyTo(array);

        int in[IMAGE_PIXELS*3];

        //workedImage.data

        for (int y = 0; y < IMAGE_HEIGHT; ++y) {
            for (int x = 0; x < IMAGE_WIDTH; ++x) {
                Vec3b pixel = workedImage.at<Vec3b>(y, x);

                in[(y*IMAGE_WIDTH + x)*3 + 0] = pixel[0];
                in[(y*IMAGE_WIDTH + x)*3 + 1] = pixel[1];
                in[(y*IMAGE_WIDTH + x)*3 + 2] = pixel[2];
            }
        }

        //cout << (int) workedImage.data[0] << endl;

        int out[IMAGE_PIXELS];

        Buffer buffer_in(context, CL_MEM_READ_ONLY, sizeof(int)*IMAGE_PIXELS*3);
        Buffer buffer_out(context, CL_MEM_WRITE_ONLY, sizeof(int)*IMAGE_PIXELS);
        //Buffer buffer_colors(context, CL_MEM_READ_ONLY, sizeof(int)*NUMBER_OF_COLORS*3);
        Buffer buffer_colors(context, CL_MEM_READ_ONLY, sizeof(int)*NUMBER_OF_COLORS*6);

        Kernel kernel(program, "mark_pixels");
        kernel.setArg(0, buffer_in);
        kernel.setArg(1, buffer_out);
        kernel.setArg(2, buffer_colors);

        /// MARK PIXELS
        CommandQueue queue(context, devices[0]);
        queue.enqueueWriteBuffer(buffer_in, CL_TRUE, 0, sizeof(int)*IMAGE_PIXELS*3, in);
        //queue.enqueueWriteBuffer(buffer_colors, CL_TRUE, 0, sizeof(int)*NUMBER_OF_COLORS*3, colorMeans);
        queue.enqueueWriteBuffer(buffer_colors, CL_TRUE, 0, sizeof(int)*NUMBER_OF_COLORS*6, colorRanges);
        queue.enqueueNDRangeKernel(kernel, NullRange, NDRange(IMAGE_WIDTH, IMAGE_HEIGHT), NullRange);
        queue.enqueueReadBuffer(buffer_out, CL_TRUE, 0, sizeof(int)*IMAGE_PIXELS, out);
        queue.finish();

        // Test
        if (testingMode) {
            for (int y = 0; y < IMAGE_HEIGHT; ++y) {
                for (int x = 0; x < IMAGE_WIDTH; ++x) {
                    switch (out[y * IMAGE_WIDTH + x]) {
                        // white
                        case 0:
                            image.at<Vec3b>(y, x) = Vec3b(255, 255, 255);
                            //workedImage.at<Vec3b>(y, x) = colorMap["WHITE"];
                            break;
                            // yellow
                        case 1:
                            image.at<Vec3b>(y, x) = Vec3b(0, 255, 255);
                            //workedImage.at<Vec3b>(y, x) = colorMap["YELLOW"];
                            break;
                            // blue
                        case 2:
                            image.at<Vec3b>(y, x) = Vec3b(255, 0, 0);
                            //workedImage.at<Vec3b>(y, x) = colorMap["BLUE"];
                            break;
                            // black
                        case 3:
                            image.at<Vec3b>(y, x) = Vec3b(0, 0, 0);
                            //workedImage.at<Vec3b>(y, x) = colorMap["BLACK"];
                            break;
                            // green
                        case 4:
                            image.at<Vec3b>(y, x) = Vec3b(0, 255, 0);
                            //workedImage.at<Vec3b>(y, x) = colorMap["GREEN"];
                            break;
                            // orange
                        case 5:
                            image.at<Vec3b>(y, x) = Vec3b(0, 165, 255);
                            //image.at<Vec3b>(y, x) = Vec3b()
                            break;
                    }
                }
            }
        }

        /// DETECT BLOBS OF SAME COLOR
        vector<Blob> blobs;
        vector<BlobLine> previousLines;

        for (int y = 0; y < IMAGE_HEIGHT; ++y) {
            vector<BlobLine> currentLines;

            int lastColor = out[y*IMAGE_WIDTH];
            int beginColorIndex = 0;

            for (int x = 1; x < IMAGE_WIDTH + 1; ++x) {
                int i = y*IMAGE_WIDTH + x;
                int currentColor = (x == IMAGE_WIDTH) ? -1 : out[i];

                if (lastColor != currentColor) {
                    if ((lastColor == 1 || lastColor == 2 || lastColor == 5) && x - 1 - beginColorIndex > 1) {//} || out[i] == 2) {
                        BlobLine line;
                        line.y = y;
                        line.xi = beginColorIndex;
                        line.xf = x - 1;
                        line.color = lastColor;
                        line.blobIndex = -1;

                        for (int l = 0; l < previousLines.size(); ++l) {
                            // Skip if colors do not match
                            if (previousLines[l].color != line.color) {
                                continue;
                            }

                            /*
                            // Skip if lines do not touch
                            if (previousLines[l].xi > line.xf || previousLines[l].xf < line.xi) {
                                continue;
                            }
                             */

                            // Skip if touching is too small
                            int common = min(previousLines[l].xf, line.xf) - max(previousLines[l].xi, line.xi);

                            if (common < 2) {
                                continue;
                            }

                            // Connect lines into blob
                            if (line.blobIndex == -1) {
                                line.blobIndex = previousLines[l].blobIndex;
                                //cout << line.blobIndex << endl;
                            } else if (line.blobIndex != previousLines[l].blobIndex) {
                                blobs[line.blobIndex].addBlob(blobs[previousLines[l].blobIndex]);
                                blobs[previousLines[l].blobIndex].mHidden = true;
                                previousLines[l].blobIndex = line.blobIndex;
                            }
                        }

                        // Create new blob?
                        if (line.blobIndex == -1) {
                            Blob blob(lastColor);
                            line.blobIndex = (int) blobs.size();
                            blobs.push_back(blob);
                        }

                        // Add line to blob
                        blobs[line.blobIndex].addLine(line);

                        // Add to current row
                        currentLines.push_back(line);
                    }

                    beginColorIndex = x;
                    lastColor = currentColor;
                }
            }

            previousLines = currentLines;
        }

        /// FIND BALLS AND GOALS
        vector<Detector::Ball> initialBalls;

        Point ownGoalCenter;
        Point opponentGoalCenter;

        int largestOwnGoalSurface = 1500;
        int largestOpponentGoalSurface = 1500;
        int opponentGoalWidth;

        for (int i = 0; i < blobs.size(); ++i) {
            if (blobs[i].mHidden) {
                continue;
            }

            // Blob parameters
            int w = blobs[i].getWidth();
            int h = blobs[i].getHeight();
            int A = blobs[i].getSurface();

            Point center = blobs[i].getMassCenter();

            // Remove noise
            if (h <= 1) {
                continue;
            }

            // Get blob size in centimeters
            FloatPoint leftTop = detector.getDistance(Point(blobs[i].mMinX, blobs[i].mMinY));
            FloatPoint leftBottom = detector.getDistance(Point(blobs[i].mMinX, blobs[i].mMaxY));
            FloatPoint rightBottom = detector.getDistance(Point(blobs[i].mMaxX, blobs[i].mMaxY));

            Point size = Point(
                    (int) round(rightBottom.x - leftBottom.x),
                    (int) round(leftTop.y - leftBottom.y)
            );

            //putText(image, itos(size.y), Point(center.x + 20, center.y), 1, 1, Scalar(0, 0, 255));

            // Check if blob is a ball
            if (blobs[i].mColor == 5 && size.x >= 1 && size.x <= 10 && size.y > 1 && size.y < 100) {
                /*
                int greenNeighbours = 0;

                for (int j = 1; j < 2; ++j) {
                    if (out[(blobs[i].mMinY - j) * IMAGE_WIDTH + center.x] == 4) {
                        ++greenNeighbours;
                    }

                    if (out[(blobs[i].mMaxY + j) * IMAGE_WIDTH + center.x] == 4) {
                        ++greenNeighbours;
                    }

                    if (out[center.y * IMAGE_WIDTH + (blobs[i].mMinX - j)] == 4) {
                        ++greenNeighbours;
                    }

                    if (out[center.y * IMAGE_WIDTH + (blobs[i].mMaxX + j)] == 4) {
                        ++greenNeighbours;
                    }
                }
                 */

                //if (greenNeighbours > 1) {
                    Detector::Ball ball;
                    ball.center = center;
                    ball.distance = detector.getDistance(Point(center.x, blobs[i].mMaxY));
                    initialBalls.push_back(ball);
                    //continue;
                //}
            }

            // Check if blob is a goal
            else if ((blobs[i].mColor == opponentGoalColor || blobs[i].mColor == ownGoalColor) && h > 20 && h < 200 && size.x > 10) {
                // Check size
                if (blobs[i].mColor == opponentGoalColor && A < largestOpponentGoalSurface) {
                    continue;
                }

                if (blobs[i].mColor == ownGoalColor && A < largestOwnGoalSurface) {
                    continue;
                }

                // It is probably not the goal and the blob is a robot if other goal color is directly above or below
                int otherColorNeighbours = 0;

                for (int j = 0; j < (blobs[i].mMaxX - center.y + 5); ++j) {
                    if (out[(center.y - j) * IMAGE_WIDTH + center.x] == ownGoalColor) {
                        ++otherColorNeighbours;
                    }

                    if (out[(center.y + j) * IMAGE_WIDTH + center.x] == ownGoalColor) {
                        ++otherColorNeighbours;
                    }
                }

                if (otherColorNeighbours < (blobs[i].mMaxX - center.y)/2) {
                    // Mark as current goal
                    if (blobs[i].mColor == ownGoalColor) {
                        largestOwnGoalSurface = A;
                        ownGoalCenter = Point(center.x, blobs[i].mMaxY);
                    } else {
                        largestOpponentGoalSurface = A;
                        opponentGoalCenter = Point(center.x, blobs[i].mMaxY);
                        opponentGoalWidth = size.x;
                    }

                    putText(image, itos(A) + " " + itos(h), Point(center.x + 20, center.y), 1, 1, Scalar(0, 0, 0));

                    line(image, Point(blobs[i].mMinX, blobs[i].mMinY), Point(blobs[i].mMaxX, blobs[i].mMinY),
                         Scalar(0, 0, 0));
                    line(image, Point(blobs[i].mMinX, blobs[i].mMaxY), Point(blobs[i].mMaxX, blobs[i].mMaxY),
                         Scalar(0, 0, 0));
                    line(image, Point(blobs[i].mMinX, blobs[i].mMinY), Point(blobs[i].mMinX, blobs[i].mMaxY),
                         Scalar(0, 0, 0));
                    line(image, Point(blobs[i].mMaxX, blobs[i].mMinY), Point(blobs[i].mMaxX, blobs[i].mMaxY),
                         Scalar(0, 0, 0));

                    //continue;
                }
            }

            // Unknown
            else {
                //continue;
                putText(image, itos(h), Point(center.x + 20, center.y), 1, 1, Scalar(0, 0, 0));

                /*
                line(image, Point(blobs[i].mMinX, blobs[i].mMinY), Point(blobs[i].mMaxX, blobs[i].mMinY),
                     Scalar(0, 0, 0));
                line(image, Point(blobs[i].mMinX, blobs[i].mMaxY), Point(blobs[i].mMaxX, blobs[i].mMaxY),
                     Scalar(0, 0, 0));
                line(image, Point(blobs[i].mMinX, blobs[i].mMinY), Point(blobs[i].mMinX, blobs[i].mMaxY),
                     Scalar(0, 0, 0));
                line(image, Point(blobs[i].mMaxX, blobs[i].mMinY), Point(blobs[i].mMaxX, blobs[i].mMaxY),
                     Scalar(0, 0, 0));
                     */
            }
        }

        // Filter actual balls
        vector<Detector::Ball> balls;

        for (int i = 0; i < initialBalls.size(); ++i) {
            Detector::Ball ball = initialBalls[i];

            if (!detector.isBallWithinBorders(out, ball)) {
                continue;
            }

            balls.push_back(ball);
        }

        /// TRY TO FIND BORDER IN FORWARD DIRECTION
        int borderY = detector.findBorder(out, IMAGE_HALF_WIDTH);

        /// TESTS
        // Draw goal line for testing
        if (opponentGoalCenter.x && opponentGoalCenter.y) {
            line(image, Point(opponentGoalCenter.x, 0), Point(opponentGoalCenter.x, IMAGE_HEIGHT), Scalar(0, 0, 255), 1);
            line(image, Point(0, opponentGoalCenter.y), Point(IMAGE_WIDTH, opponentGoalCenter.y), Scalar(0, 0, 255), 1);
        }

        // Draw center line for testing
        line(image, Point(IMAGE_HALF_WIDTH, 0), Point(IMAGE_HALF_WIDTH, IMAGE_HEIGHT), Scalar(255, 0, 255), 1);

        // Draw border line for testing
        if (borderY != -1) {
            line(image, Point(0, borderY), Point(IMAGE_WIDTH, borderY), Scalar(0, 0, 0));
        }

        // Draw balls for testing
        for (int i = 0; i < balls.size(); ++i) {
            putText(image, itos((int) round(balls[i].distance.y)), Point(balls[i].center.x + 20, balls[i].center.y), 1, 1, Scalar(0, 0, 255));
            circle(image, balls[i].center, 20, Scalar(0, 0, 255));
        }

        /// REFEREE COMMANDS
        string refereeCommand = srf.getRefereeCommand();

        if (refereeCommand == "START") {
            gameIsOn = true;
        } else if (refereeCommand == "STOP") {
            gameIsOn = false;
        }

        /// NOTIFY AI OF CURRENT STATE
        ai.notify(gameIsOn, balls, communicator.isBallCaptured(), opponentGoalCenter, ownGoalCenter, opponentGoalWidth);

        /// ASK AI WHAT TO DO
        gettimeofday(&tp, NULL);
        long int time = tp.tv_sec * 1000 + tp.tv_usec / 1000;
        string command = ai.getCommand((int) (time - startTime));
        putText(image, itos((int) (time - startTime)), Point(20, 20), 1, 1, Scalar(0, 255, 255));
        startTime = time;

        if (command.length())
        {
            //communicator.sendCommand("red");
            communicator.sendCommand(command);
            //cout << command << endl;
            //cout << communicator.isBallCaptured() << endl;
        }

        //cvtColor(workedImage, workedImage, COLOR_HSV2BGR);

        imshow("test", image);

        // Close when pressing space or esc
        if (waitKey(30) > 0) {
            break;
        }
    }

    communicator.sendCommand("sd0:0:0:0");
    communicator.sendCommand("d0");

    return 0;
}