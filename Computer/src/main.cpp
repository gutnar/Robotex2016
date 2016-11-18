#include <opencv2/highgui.hpp>
#include <SimpleIni.h>
#include <sys/time.h>
#include "omp.h"


#include "Communicator.h"
#include "Calibrator.h"
#include "Detector.h"
#include "AI.h"

using namespace cv;
using namespace std;

int main() {

    /// LOAD CONFIGURATION
    CSimpleIniA configuration;
    configuration.SetUnicode();
    configuration.LoadFile("configuration.ini");

    /// CREATE SRF COMMUNICATOR
    Communicator srf;

    try {
        /*
        int vendorId = atoi(configuration.GetValue("srf", "VENDOR_ID", "0"));
        int productId = atoi(configuration.GetValue("srf", "PRODUCT_ID", "0"));
        srf.connect(vendorId, productId);
         */
        srf.connect("/dev/ttyACM0");
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
        communicator.connect("/dev/ttyACM2");
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
    bool gameIsOn = false;

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

    /// MAIN LOOP
    while (true) {
        /// IMAGE MANIPULATION
        Mat image, workedImage;
        cap >> image;

        //namedWindow("cam");
        //imshow("cam", image);

        //cout << image.cols << endl;

        // Convert BGR to HSV
        cvtColor(image, workedImage, COLOR_BGR2HSV);

        // Pixels of ball color
        vector<Point> ballPixels;

        /// MARK PIXELS
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
                        closestColor = "YELLOW";

                        // Remember ball pixel
                        ballPixels.push_back(Point(x, y));
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
                else if (pixel[2] >= 140) {
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

        /// FIND BALLS


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
        */

        /// ASK AI WHAT TO DO
        gettimeofday(&tp, NULL);
        long int time = tp.tv_sec * 1000 + tp.tv_usec / 1000;
        string command = ai.getCommand(time - startTime);
        Scalar white = Scalar(255, 255, 255);
        putText(workedImage, itos(time - startTime), Point(20, 20), 1, 1, white);
        startTime = time;

        /*
        if (command.length())
        {
            //communicator.sendCommand("red");
            communicator.sendCommand(command);
        }
         */

        //cvtColor(workedImage, workedImage, COLOR_HSV2BGR);
        imshow("test", workedImage);

        // Close when pressing space or esc
        if (waitKey(30) > 0) {
            break;
        }
    }

    communicator.sendCommand("sd0:0:0:0");
    communicator.sendCommand("d0");

    return 0;
}