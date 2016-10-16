//
// Created by Gutnar Leede on 01/10/16.
//

#ifndef ROBOTEX2016_DETECTOR_H
#define ROBOTEX2016_DETECTOR_H

#include <opencv2/opencv.hpp>
#include <SimpleIni.h>

using namespace std;
using namespace cv;

class Detector {
public:
    // Ball structure
    struct Ball {
        Point center;
        int radius;
    };

    // Constructor declaration
    Detector(CSimpleIniA &configurationIni, CSimpleIniA &colorsIni);
    void filterColor(Mat &srcImage, Mat &dstImage, string color);
    vector<vector<Point> > findGoal(Mat &srcImage, string color);
    vector<Ball> findBalls(Mat &srcImage);
private:
    CSimpleIniA *mConfigurationIni;
    CSimpleIniA *mColorsIni;
};


#endif //ROBOTEX2016_DETECTOR_H
