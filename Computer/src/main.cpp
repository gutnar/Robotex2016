#include <opencv2/highgui.hpp>
#include <SimpleIni.h>
#include <libserialport.h>

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

    /// CREATE SERIAL COMMUNICATOR
    int vendorId = atoi(configuration.GetValue("settings", "VENDOR_ID", "0"));
    int productId = atoi(configuration.GetValue("settings", "PRODUCT_ID", "0"));

    Communicator communicator(vendorId, productId);

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
    namedWindow("BALLS");

    /// SET UP AI
    AI ai;

    /// MAIN LOOP
    while (true) {
        /// IMAGE MANIPULATION
        Mat image, workedImage;
        cap >> image;

        //cout << image.cols << endl;

        // Convert BGR to HSV
        cvtColor(image, workedImage, COLOR_BGR2HSV);

        /// FIND BALLS
        vector<Detector::Ball> balls = detector.findBalls(workedImage);

        /// FIND GOALS
        //vector<vector<Point> > contours = detector.findGoal(workedImage, configuration.GetValue("settings", "GOAL_COLOR", NULL));

        /// NOTIFY AI OF CURRENT STATE
        ai.notifyPositions(balls);

        /// ASK AI WHAT TO DO
        string command = ai.getCommand();

        cout << command << endl;

        if (command.length()) {
            communicator.sendCommand(command);
        }

        /// DRAW FOR TESTING
        Scalar red = Scalar(0, 0, 255);
        Scalar blue = Scalar(255, 0, 0);

        for (int i = 0; i < balls.size(); ++i) {
            circle(image, balls[i].center, 2, red);
            circle(image, balls[i].center, balls[i].radius, red, 3);
            //circle(image, balls[i].center, sqrt(maxRR), blue, 3);
            //circle(srcImage,  center, 2, red);
            //circle(srcImage,  center, 2, red);
        }

        /*
        for (int i = 0; i < contours.size(); ++i) {
            drawContours(image, contours, i, blue, 3);
        }
         */

        imshow("BALLS", image);

        // Close when pressing space or esc
        if (waitKey(30) > 0) {
            break;
        }
    }

    return 0;
}