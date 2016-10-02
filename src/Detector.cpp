//
// Created by Gutnar Leede on 01/10/16.
//

#include "Detector.h"

using namespace std;
using namespace cv;

//Constructor
Detector::Detector(CSimpleIniA *configurationIni, CSimpleIniA *colorsIni) {
    //Pointers
    mConfigurationIni = configurationIni;
    mColorsIni  = colorsIni;
};

void Detector::filterColor(Mat srcImage, Mat dstImage, string color) {
    // Load color data
    int values[6];
    string keys[6] = {"H_MIN","H_MAX","S_MIN","S_MAX","V_MIN","V_MAX"};

    for (int i = 0; i < 6; ++i) {
        values[i] = atoi(mColorsIni->GetValue(color.c_str(), keys[i].c_str(), NULL));
    }

    // Filters workedImage according to a color
    inRange(srcImage, Scalar(values[0], values[2], 0), Scalar(values[1], values[3], 255), dstImage);

    // Erode
    erode(dstImage, dstImage, getStructuringElement(MORPH_ELLIPSE, Size(10, 10), Point(0, 0)));
}

void Detector::findBalls(Mat srcImage) {
    Mat filteredImage;
    filterColor(srcImage, filteredImage, "ORANGE");
    cout << "SIIN"<< endl;
    //check if works
    namedWindow("BALLS");
    imshow("BALLS", filteredImage);

}