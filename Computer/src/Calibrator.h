//
// Created by jk on 20.09.16.
//

#ifndef ROBOTEX2016_CALIBRATOR_H
#define ROBOTEX2016_CALIBRATOR_H

#include <string>
#include <vector>
#include <opencv2/opencv.hpp>
#include <SimpleIni.h>

#include "common.h"

using namespace std;
using namespace cv;

class Calibrator {
public:
    void calibrate(VideoCapture cap, CSimpleIniA *ini);
    void calibrateColor(VideoCapture cap, CSimpleIniA *ini, string color);
    void onMouse(int event, int x, int y);
    static void mouseEventHandler(int event, int x, int y, int, void* userdata);
    void setCalibrationSrc(Mat image);
private:
    Mat mCalibrationSrc;
    bool mCalibrationSrcOverwritten;
    vector< vector<Point> > mPolygons;
    string mWindowName;
};


#endif //ROBOTEX2016_CALIBRATOR_H
