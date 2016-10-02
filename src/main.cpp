#include <opencv2/highgui.hpp>
#include <SimpleIni.h>

#include "Calibrator.h"
#include "Detector.h"

using namespace cv;
using namespace std;

int main() {
    VideoCapture cap;
    cap.open(0);

    // Load configuration
    CSimpleIniA configuration;
    configuration.SetUnicode();
    configuration.LoadFile("configuration.ini");
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

    // Detection detector(configuation file, colors file)
    //Call for constructor
    Detector detector(configuration, colors);

    namedWindow("BALLS");

    while (true) {
        /// IMAGE MANIPULATION
        Mat image, workedImage;
        cap >> image;

        // Convert BGR to HSV
        cvtColor(image, workedImage, COLOR_BGR2HSV);

        // Smooth the image
        GaussianBlur(workedImage, workedImage, Size(25, 25), 2, 2);

        /// FIND BALLS
        vector<Detector::Ball> balls = detector.findBalls(workedImage);

        /// FIND GOALS
        vector<vector<Point> > contours = detector.findGoal(workedImage, configuration.GetValue("settings", "GOAL_COLOR", NULL));

        /// DRAW
        // Draw circles check
        Scalar red = Scalar(0, 0, 255);
        Scalar blue = Scalar(255, 0, 0);

        cout << balls.size() << endl;

        for (int i = 0; i < balls.size(); ++i) {
            circle(image, balls[i].center, 2, red);
            circle(image, balls[i].center, balls[i].radius, red, 3);
            //circle(image, balls[i].center, sqrt(maxRR), blue, 3);
            //circle(srcImage,  center, 2, red);
            //circle(srcImage,  center, 2, red);
        }

        for (int i = 0; i < contours.size(); ++i) {
            drawContours(image, contours, i, blue);
        }

        imshow("BALLS", image);

        // Close when pressing space or esc
        if (waitKey(30) > 0) {
            break;
        }
    }

    return 0;
}