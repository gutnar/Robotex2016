#include <opencv2/highgui.hpp>
#include <SimpleIni.h>
#include <sys/time.h>

#include "Communicator.h"
#include "Calibrator.h"
#include "Detector.h"
#include "AI.h"

using namespace cv;
using namespace std;

int main()
{
    /// LOAD CONFIGURATION
    CSimpleIniA configuration;
    configuration.SetUnicode();
    configuration.LoadFile("configuration.ini");

    /// CREATE SRF COMMUNICATOR
    Communicator srf;

    try
    {
        /*
        int vendorId = atoi(configuration.GetValue("srf", "VENDOR_ID", "0"));
        int productId = atoi(configuration.GetValue("srf", "PRODUCT_ID", "0"));
        srf.connect(vendorId, productId);
         */
        srf.connect("/dev/ttyACM0");
    } catch (int exception)
    {
        cout << "Could not create serial RF connection!" << endl;
        //return 0;
    }

    /// CREATE SERIAL MOTHERBOARD COMMUNICATOR
    Communicator communicator;

    try
    {
        /*
        int vendorId = atoi(configuration.GetValue("motherboard", "VENDOR_ID", "0"));
        int productId = atoi(configuration.GetValue("motherboard", "PRODUCT_ID", "0"));
        communicator.connect(vendorId, productId);
         */
        communicator.connect("/dev/ttyACM1");
    } catch (int exception)
    {
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
    if (rc < 0)
    {
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

    /// MAIN LOOP
    while (true)
    {
        /// IMAGE MANIPULATION
        Mat image, workedImage;
        cap >> image;

        //namedWindow("cam");
        //imshow("cam", image);

        //cout << image.cols << endl;

        // Convert BGR to HSV
        cvtColor(image, workedImage, COLOR_BGR2HSV);

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

        // Close when pressing space or esc
        if (waitKey(30) > 0)
        {
            break;
        }
    }

    communicator.sendCommand("sd0:0:0:0");
    communicator.sendCommand("d0");

    return 0;
}