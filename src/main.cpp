#include <opencv2/highgui.hpp>
#include <simpleini.h>

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
    Detector detector(&configuration, &colors);

    // IMAGE MANIPULATION
    Mat image, workedImage;
    cap >> image;

    // Convert BGR to HSV
    cvtColor(image, workedImage, COLOR_BGR2HSV);

    // Smooth the image
    GaussianBlur(workedImage, workedImage, Size(25, 25), 2, 2);

    //Call for filterColor
    //detector.filterColor(workedImage, "ORANGE");
    detector.findBalls(workedImage);

    return 0;
}