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

int main3()
{
    int x = 400;
    int y = 30;
    int z = 20;

    // GPU 3d loop
    vector<Platform> platforms;
    vector<Device> devices;
    vector<Kernel> kernels;

    // create platform, context and command queue
    Platform::get(&platforms);
    platforms[0].getDevices(CL_DEVICE_TYPE_GPU, &devices);
    Context context(devices);
    CommandQueue queue(context, devices[0]);

    // load opencl source
    std::ifstream cl_file("src/kernels.cl");
    std::string cl_string(std::istreambuf_iterator<char>(cl_file),
                          (std::istreambuf_iterator<char>()));
    Program::Sources source(1, std::make_pair(cl_string.c_str(),
                                              cl_string.length() + 1));

    // create program and kernel and set kernel arguments
    Program program(context, source);
    program.build(devices);
    Kernel kernel(program, "ndrange_parallelism");

    // execute kernel and wait for completion
    NDRange global_work_size(x, y, z);
    queue.enqueueNDRangeKernel(kernel, NullRange, global_work_size, NullRange);
    queue.finish();

    return 0;
}

//#define PIXEL_FORMAT V4L2_PIX_FMT_YUYV

int main2()
{
    /// START VIDEO CAPTURE
    //VideoCapture cvCap;
    //cvCap.open(0);

    /*
    /// MAIN LOOP
    while (true) {
        /// IMAGE MANIPULATION
        cvCap >> image;

        //circle(image, Point(320, 240), 2, Scalar(255, 0, 255));
        imshow("test", image);

        // Close when pressing space or esc
        if (waitKey(30) > 0) {
            break;
        }
    }

    cvCap.release();
     */

    /*
    circle(image, Point(320, 240), 2, Scalar(255, 0, 255));
    imshow("test", image);
     */

    namedWindow("test");

    Capture cap;
    LowVision vision;

    const char *video_device = "/dev/video0";
    const int input_idx = 1;
    const int width  = IMAGE_WIDTH;
    const int height = IMAGE_HEIGHT;

    // initialize
    cap.init(video_device, input_idx, width, height, V4L2_PIX_FMT_YUYV);
    char tmap_file[64];
    snprintf(tmap_file,64,"config/thresh.%d%d%d.tmap.gz",bits_y,bits_u,bits_v);
    vision.init("config/colors.txt",tmap_file,width,height);

    // main loop
    while (true) {
        // capture and process a frame
        const Capture::Image *img = cap.captureFrame();

        if (img != NULL) {
            vision_image cmv_img;
            cmv_img.buf    = (pixel*)(img->data);
            cmv_img.width  = img->width;
            cmv_img.height = img->height;
            cmv_img.pitch  = img->bytesperline;
            cmv_img.field  = img->field;

            vision.processFrame(cmv_img);

            //Mat image(IMAGE_WIDTH, IMAGE_HEIGHT, CV_8UC3, Scalar(0, 0, 0));
            Mat image = Mat::zeros(IMAGE_WIDTH, IMAGE_HEIGHT, CV_8U);

            const Region* region = NULL;

            for (region = vision.getRegions(1); region != NULL; region = region->next) {
                circle(image, Point(region->x1, region->y1), 5, Scalar(255, 0, 255));
            }

            //cout << vision.getRegions(1)->next->area << endl;

            cap.releaseFrame(img);

            imshow("test", image);
        } else {
            break;
        }

        if (waitKey(30) > 0) {
            break;
        }
    }

    // shutdown
    vision.close();
    cap.close();

    return(0);
}

int main() {
    /// LOAD CONFIGURATION
    CSimpleIniA configuration;
    configuration.SetUnicode();
    configuration.LoadFile("configuration.ini");

    /// CREATE SRF COMMUNICATOR
    Communicator srf(configuration.GetValue("settings", "FIELD_ID")[0], configuration.GetValue("settings", "ROBOT_ID")[0]);

    try {
        /*
        int vendorId = atoi(configuration.GetValue("srf", "VENDOR_ID", "0"));
        int productId = atoi(configuration.GetValue("srf", "PRODUCT_ID", "0"));
        srf.connect(vendorId, productId);
         */
        srf.connect("/dev/ttyACM1");
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
    AI ai;

    // GAME STATUS
    bool gameIsOn = true;

    // TIMESTAMP
    struct timeval tp;
    gettimeofday(&tp, NULL);
    long int startTime = tp.tv_sec * 1000 + tp.tv_usec / 1000;

    // Start charging
    communicator.sendCommand("c1");

    // Load color data
    map<string, Vec3b> colorMap;
    string yellowishColors[2] = {"ORANGE", "YELLOW"};
    string bluishColors[3] = {"BLUE", "GREEN", "BLACK"};
    string keys[6] = {"H_MIN", "H_MAX", "S_MIN", "S_MAX", "V_MIN", "V_MAX"};

    for (int i = 0; i < NUMBER_OF_COLORS; ++i) {
        int colorMeans[3];

        for (int j = 0; j < 6; j += 2) {
            colorMeans[j / 2] = (atoi(colorsIni.GetValue(COLORS[i].c_str(), keys[j].c_str(), NULL)) +
                                 atoi(colorsIni.GetValue(COLORS[i].c_str(), keys[j + 1].c_str(), NULL))) / 2;
        }

        colorMap.insert(pair<string, Vec3b>(COLORS[i], Vec3b((uchar) colorMeans[0], (uchar) colorMeans[1],
                                                                   (uchar) colorMeans[2])));
    }

    /*
    int values[6];
    string keys[6] = {"H_MIN","H_MAX","S_MIN","S_MAX","V_MIN","V_MAX"};

    for (int i = 0; i < 6; ++i) {
        values[i] = atoi(colors.GetValue(color.c_str(), keys[i].c_str(), NULL));
    }*/

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

        //cout << image.cols << endl;

        // Convert BGR to HSV
        cvtColor(image, workedImage, COLOR_BGR2HSV);

        // Pixels of ball color
        //vector<Point> ballPixels;

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

        Kernel kernel(program, "mark_pixels");
        kernel.setArg(0, buffer_in);
        kernel.setArg(1, buffer_out);

        /// MARK PIXELS
        CommandQueue queue(context, devices[0]);
        queue.enqueueWriteBuffer(buffer_in, CL_TRUE, 0, sizeof(int)*IMAGE_PIXELS*3, in);
        queue.enqueueNDRangeKernel(kernel, NullRange, NDRange(IMAGE_WIDTH, IMAGE_HEIGHT), NullRange);
        queue.enqueueReadBuffer(buffer_out, CL_TRUE, 0, sizeof(int)*IMAGE_PIXELS, out);
        queue.finish();

        // Test
        for (int y = 0; y < IMAGE_HEIGHT; ++y) {
            for (int x = 0; x < IMAGE_WIDTH; ++x) {
                switch (out[y*IMAGE_WIDTH + x]) {
                    // white
                    case 0:
                        image.at<Vec3b>(y, x) = Vec3b(255, 255, 255);
                        //workedImage.at<Vec3b>(y, x) = colorMap["WHITE"];
                        break;
                    // yellow
                    case 1:
                        image.at<Vec3b>(y, x) = Vec3b(0, 255, 255);
                        //workedImage.at<Vec3b>(y, x) = colorMap["ORANGE"];
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
                    if ((lastColor == 1 || lastColor == 2) && x - 1 - beginColorIndex > 1) {//} || out[i] == 2) {
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

                            // Skip if lines do not touch
                            if (previousLines[l].xi > line.xf || previousLines[l].xf < line.xi) {
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
                            Blob blob;
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

        // Test
        for (int i = 0; i < blobs.size(); ++i) {
            //image.at<Vec3b>(lines[i][0], lines[i][1]) = Vec3b(0, 0, 255);
            //image.at<Vec3b>(lines[i][0], lines[i][2]) = Vec3b(0, 0, 255);
            if (blobs[i].mMinY > IMAGE_HEIGHT/2 && blobs[i].mSurface < 50){
                blobs[i].mHidden = true;
            }

            if (!blobs[i].mHidden) {
                line(image, Point(blobs[i].mMinX, blobs[i].mMinY), Point(blobs[i].mMaxX, blobs[i].mMinY),
                     Scalar(0, 0, 255));
                line(image, Point(blobs[i].mMinX, blobs[i].mMaxY), Point(blobs[i].mMaxX, blobs[i].mMaxY),
                     Scalar(0, 0, 255));
                line(image, Point(blobs[i].mMinX, blobs[i].mMinY), Point(blobs[i].mMinX, blobs[i].mMaxY),
                     Scalar(0, 0, 255));
                line(image, Point(blobs[i].mMaxX, blobs[i].mMinY), Point(blobs[i].mMaxX, blobs[i].mMaxY),
                     Scalar(0, 0, 255));
            }
        }

        /*
#pragma omp parallel for
        for (int y = 0; y < IMAGE_HEIGHT; ++y) {
            for (int x = 0; x < IMAGE_WIDTH; ++x) {
                Vec3b pixel = workedImage.at<Vec3b>(y, x);
                string closestColor = "WHITE";
                float minDifference = 3;
                for (auto &color : colorMap) {
                    float difference =
                            pow((float) (pixel[0] - color.second[0]) / 180, 2) +
                            pow((float) (pixel[1] - color.second[1]) / 255, 2) +
                            pow((float) (pixel[2] - color.second[2]) / 255, 2);

                    if (difference < minDifference) {
                        minDifference = difference;
                        closestColor = color.first;
                    }
                }

                image.at<Vec3b>(y, x) = colorMap[closestColor];
            }
        }
         */

        /// FPS
        gettimeofday(&tp, NULL);
        long int time = tp.tv_sec * 1000 + tp.tv_usec / 1000;
        Scalar white = Scalar(255, 255, 255);
        putText(image, itos(time - startTime), Point(20, 20), 1, 1, white);
        startTime = time;

        imshow("test", image);

#ifdef TEST
#pragma omp parallel for
        for (int y = 0; y < IMAGE_HEIGHT; ++y) {
            for (int x = 0; x < IMAGE_WIDTH; ++x) {
                Vec3b pixel = workedImage.at<Vec3b>(y, x);
                string closestColor = "WHITE";

                // Light color
                if (pixel[2] >= 200) {
                    // Most likely yellow or orange
                    if (pixel[0] < 100) {
                        /*
                        float yellowDifference =
                                pow((float) (pixel[0] - colorMap["YELLOW"][0]) / 180, 2) +
                                pow((float) (pixel[1] - colorMap["YELLOW"][1]) / 255, 2) +
                                pow((float) (pixel[2] - colorMap["YELLOW"][2]) / 255, 2);

                        float orangeDifference =
                                pow((float) (pixel[0] - colorMap["ORANGE"][0]) / 180, 2) +
                                pow((float) (pixel[1] - colorMap["ORANGE"][1]) / 255, 2) +
                                pow((float) (pixel[2] - colorMap["ORANGE"][2]) / 255, 2);

                        if (yellowDifference < orangeDifference) {
                            closestColor = "YELLOW";
                        } else {
                            closestColor = "ORANGE";
                        }
                         */
                        closestColor = "ORANGE";

                        // Remember ball pixel
                        //ballPixels.push_back(Point(x, y));
                    }
                }

                // Most likely black, could be blue or green
                else if (pixel[2] <= 50) {
                    if (pixel[2] >= 40 && pixel[1] >= 175) {
                        closestColor = "BLUE";
                    } else {
                        closestColor = "BLACK";
                    }
                }

                // Most likely blue
                else if (pixel[0] >= 100 && pixel[0] <= 130 && pixel[1] >= 140) {
                    closestColor = "BLUE";
                }

                // Most likely green
                else {
                    closestColor = "GREEN";
                }

                float minDifference = 3;


                /*
                string *colors;
                int loop;

                if (pixel[2] > 200) {

                    for (int i = 0; i < 2; ++i) {
                        Vec3b color = colorMap[yellowishColors[i]];

                        float difference =
                                pow((float) (pixel[0] - color[0]) / 180, 2) +
                                pow((float) (pixel[1] - color[1]) / 255, 2) +
                                pow((float) (pixel[2] - color[2]) / 255, 2);

                        if (difference < minDifference) {
                            minDifference = difference;
                            closestColor = yellowishColors[i];
                        }
                    }
                } else {

                    for (int i = 0; i < 3; ++i) {
                        Vec3b color = colorMap[bluishColors[i]];

                        float difference =
                                pow((float) (pixel[0] - color[0]) / 180, 2) +
                                pow((float) (pixel[1] - color[1]) / 255, 2) +
                                pow((float) (pixel[2] - color[2]) / 255, 2);

                        if (difference < minDifference) {
                            minDifference = difference;
                            closestColor = bluishColors[i];
                        }
                    }
                }
                 */

                /*
                for (auto &color : colorMap) {
                    float difference =
                            pow((float) (pixel[0] - color.second[0]) / 180, 2) +
                            pow((float) (pixel[1] - color.second[1]) / 255, 2) +
                            pow((float) (pixel[2] - color.second[2]) / 255, 2);

                    if (difference < minDifference) {
                        minDifference = difference;
                        closestColor = color.first;
                    }
                }
                 */

                workedImage.at<Vec3b>(y, x) = colorMap[closestColor];
            }
        }
#endif

        /*
        /// FIND BALLS
        vector<Detector::Ball> balls = detector.findBalls(workedImage);

        /// FIND GOALS
        //vector<vector<Point> > contours = detector.findGoal(workedImage, configuration.GetValue("settings", "GOAL_COLOR", NULL));
        Point goalCenter = detector.findGoal(workedImage, configuration.GetValue("settings", "GOAL_COLOR", NULL));

        /// REFEREE COMMANDS
        string refereeCommand = srf.getRefereeCommand();

        if (refereeCommand == "START") {
            gameIsOn = true;
        } else if (refereeCommand == "STOP") {
            gameIsOn = false;
        }

        /// NOTIFY AI OF CURRENT STATE
        ai.notify(gameIsOn, balls, communicator.isBallCaptured(), goalCenter);

        /// ASK AI WHAT TO DO
        gettimeofday(&tp, NULL);
        long int time = tp.tv_sec * 1000 + tp.tv_usec / 1000;
        string command = ai.getCommand(time - startTime);
        Scalar white = Scalar(255, 255, 255);
        putText(workedImage, itos(time - startTime), Point(20, 20), 1, 1, white);
        startTime = time;

        if (command.length())
        {
            //communicator.sendCommand("red");
            communicator.sendCommand(command);
            // cout << command << endl;
        }

        //cvtColor(workedImage, workedImage, COLOR_HSV2BGR);
        imshow("test", workedImage);
         */

        // Close when pressing space or esc
        if (waitKey(30) > 0) {
            break;
        }
    }

    communicator.sendCommand("sd0:0:0:0");
    communicator.sendCommand("d0");

    return 0;
}