//
// Created by Gutnar Leede on 01/10/16.
//

#include "Detector.h"

using namespace std;
using namespace cv;

// Constructor
Detector::Detector(CSimpleIniA &configurationIni, CSimpleIniA &colorsIni) {
    //Pointers
    mConfigurationIni = &configurationIni;
    mColorsIni = &colorsIni;
};

void Detector::filterColor(Mat &srcImage, Mat &dstImage, string color) {
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

vector<Detector::Ball> Detector::findBalls(Mat &srcImage) {
    Mat filteredImage;
    filterColor(srcImage, filteredImage, "ORANGE");

    /// BALL CONTAINER
    vector<Ball> balls;

    /// FIND CONTOURS
    vector<vector<Point> > contours;
    vector<Vec4i> hierarchy;

    // Find contours from filtered image
    findContours(filteredImage, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE, Point(0, 0));

    for (int i = 0; i < contours.size(); ++i)
    {
        //drawContours(srcImage, contours, i, blue, 2, 8, hierarchy, 0, Point());
        Ball ball;

        /// FIND THE CENTER OF CONTOURS
        int sumX = 0;
        int sumY = 0;
        int contourSize = (int) contours[i].size();

        for (int j = 0; j < contourSize ; ++j) {
            sumX += contours[i][j].x;
            sumY += contours[i][j].y;
        }

        ball.center = Point(sumX/contourSize, sumY/contourSize);

        ///FIND THE BALLS' RADII
        int sumR = 0;
        // Sum of the squares
        int sumRR = 0;
        int maxRR = 0;
        int Q = 0;

        for (int j = 0; j < contourSize ; ++j) {
            int RR = pow(ball.center.x - contours[i][j].x, 2) + pow(ball.center.y - contours[i][j].y, 2);
            sumR += sqrt(RR);
            sumRR += RR;

            Q += abs(ball.center.x - contours[i][j].x) + abs(ball.center.y - contours[i][j].y);

            if (RR > maxRR) {
                maxRR = RR;
            }
        }

        // Mean
        ball.radius = sumR/contourSize;
        int deviation = sqrt(sumRR/contourSize  - pow(ball.radius, 2));

        if (ball.radius < 10) {
            continue;
        }

        if ((float) deviation/ball.radius > 0.4) {
            continue;
        }

        ball.radius = Q/contourSize;

        balls.push_back(ball);
    }

    return balls;
}

/// GOAL
vector<vector<Point> > Detector::findGoal(Mat &srcImage, string color) {
    Mat filteredImage;
    filterColor(srcImage, filteredImage, color);

    /// FIND CONTOURS
    vector<vector<Point> > contours;
    vector<Vec4i> hierarchy;

    // Find contours from filtered image
    findContours(filteredImage, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE, Point(0, 0));

    return contours;
}