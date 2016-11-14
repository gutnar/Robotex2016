//
// Created by Gutnar Leede on 01/10/16.
//

#ifndef ROBOTEX2016_DETECTOR_H
#define ROBOTEX2016_DETECTOR_H

#include <opencv2/opencv.hpp>
#include <SimpleIni.h>

#include "common.h"

using namespace std;
using namespace cv;

class Detector {
public:
    // Ball structure
    struct Ball {
        Point center;
        Point distance;
    };

    // Constructor declaration
    Detector(CSimpleIniA &configurationIni, CSimpleIniA &colorsIni);
    void filterColor(Mat &srcImage, Mat &dstImage, string color);
    Point findGoal(Mat &srcImage, string color);
    vector<Ball> findBalls(Mat &srcImage);
    bool isBallWithinBorders(Mat &srcImage, Detector::Ball ball);

    void onMouse(int event, int x, int y);
    static void mouseEventHandler(int event, int x, int y, int, void* userdata);
private:
    CSimpleIniA *mConfigurationIni;
    CSimpleIniA *mColorsIni;

    int mWhite[6];
    int mBlack[6];

    bool isPixelInColorRange(Vec3b pixel, int color[6]);
};


#endif //ROBOTEX2016_DETECTOR_H
