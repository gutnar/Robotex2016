//
// Created by Gutnar Leede on 01/10/16.
//

#ifndef ROBOTEX2016_DETECTOR_H
#define ROBOTEX2016_DETECTOR_H

#include <opencv2/opencv.hpp>
#include <simpleini.h>

using namespace std;
using namespace cv;

class Detector {
public:
    // Constructor declaration
    Detector(CSimpleIniA *configurationIni, CSimpleIniA *colorsIni);
    void filterColor(Mat srcImage, Mat dstImage, string color);
    void findBalls(Mat srcImage);
private:
    CSimpleIniA *mConfigurationIni;
    CSimpleIniA *mColorsIni;
};


#endif //ROBOTEX2016_DETECTOR_H
