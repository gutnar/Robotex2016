//
// Created by jk on 20.09.16.
//

#ifndef ROBOTEX2016_CALIBRATOR_H
#define ROBOTEX2016_CALIBRATOR_H

#include <fstream>
#include <vector>
#include <opencv2/opencv.hpp>
#include <simpleini.h>

using namespace std;
using namespace cv;

class Calibrator {
public:
    void calibrate(VideoCapture cap, CSimpleIniA *ini);
    void calibrateColor(VideoCapture cap, CSimpleIniA *ini, string color);
    void onMouse(int event, int x, int y);
    static void mouseEventHandler(int event, int x, int y, int, void* userdata);
private:
    Mat calibrationSrc;
    vector< vector<Point> > polygons;
    string windowName;
};


#endif //ROBOTEX2016_CALIBRATOR_H
