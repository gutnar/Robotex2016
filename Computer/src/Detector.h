//
// Created by Gutnar Leede on 01/10/16.
//

#ifndef ROBOTEX2016_DETECTOR_H
#define ROBOTEX2016_DETECTOR_H

#include <opencv2/opencv.hpp>
#include <SimpleIni.h>
#include <CL/cl.h>
#include <CL/cl.hpp>

#include "common.h"
#include "Blob.h"
#include "FloatPoint.h"

using namespace std;
using namespace cv;
using namespace cl;

class Detector {
public:
    // Ball structure
    struct Ball {
        Point center;
        FloatPoint distance;
    };

    Detector(CSimpleIniA &configurationIni, CSimpleIniA &colorsIni);
    void filterColor(Mat &srcImage, Mat &dstImage, string color);
    Point findGoal(Mat &srcImage, string color);
    vector<Ball> findBalls(Mat &srcImage);
    bool isBallWithinBorders(int out[IMAGE_PIXELS], Detector::Ball ball);
    int findBorder(int out[IMAGE_PIXELS], int x);
    int findOuterEdge(int out[IMAGE_PIXELS], int x);

    void onMouse(int event, int x, int y);
    static void mouseEventHandler(int event, int x, int y, int, void* userdata);

    void processImage(Mat& image);

    FloatPoint getDistance(Point point);
private:
    CSimpleIniA *mConfigurationIni;
    CSimpleIniA *mColorsIni;

    int mWhite[6];
    int mBlack[6];

    bool isPixelInColorRange(Vec3b pixel, int color[6]);

    Program mProgram;
    Context mContext;
    Device mDevice;
};


#endif //ROBOTEX2016_DETECTOR_H
