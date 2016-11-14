#include <opencv2/highgui.hpp>
#include <SimpleIni.h>
#include <sys/time.h>
#include <string>

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
        communicator.connect("/dev/ttyACM1");
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
    CSimpleIniA colors;
    colors.SetUnicode();
    SI_Error rc = colors.LoadFile(("calibration/" + place + ".ini").c_str());

    // Calibrate
    if (rc < 0) {
        Calibrator calibrator;
        calibrator.calibrate(cap, &colors);

        colors.SaveFile(("calibration/" + place + ".ini").c_str());
    }

    /// SET UP DETECTOR
    // Detection detector(configuration file, colors file)
    Detector detector(configuration, colors);
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
    string colorMapKeys[5] = {"WHITE", "BLACK", "ORANGE", "BLUE", "GREEN"};
    string keys[6] = {"H_MIN", "H_MAX", "S_MIN", "S_MAX", "V_MIN", "V_MAX"};

    for (int i = 0; i < 5; ++i) {
        int colorMeans[3];

        for (int j = 0; j < 6; j += 2) {
            colorMeans[j / 2] = (atoi(colors.GetValue(colorMapKeys[i].c_str(), keys[j].c_str(), NULL)) +
                                 atoi(colors.GetValue(colorMapKeys[i].c_str(), keys[j + 1].c_str(), NULL))) / 2;
        }

        colorMap.insert(pair<string, Vec3b>(colorMapKeys[i], Vec3b((uchar) colorMeans[0], (uchar) colorMeans[1],
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

        /*
        /// TEST
        // Mark pixels
        for (int y = 0; y < IMAGE_HEIGHT; ++y) {
            for (int x = 0; x < IMAGE_WIDTH; ++x) {
                Vec3b pixel = workedImage.at<Vec3b>(y, x);
                string closestColor = "BLACK";
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

                workedImage.at<Vec3b>(y, x) = colorMap[closestColor];
            }
        }
         */

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
        startTime = tp.tv_sec * 1000 + tp.tv_usec / 1000;

        if (command.length())
        {
            //communicator.sendCommand("red");
            communicator.sendCommand(command);
        }

        /*
        for (int x = 0; x < IMAGE_WIDTH; ++x) {
            int y = detector.findBorder(workedImage, x);

            if (y != -1) {
                for (int j = 0; j < y; ++j) {
                    image.at<Vec3b>(j, x) = Vec3b(0, 0, 0);
                }
            }
        }
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