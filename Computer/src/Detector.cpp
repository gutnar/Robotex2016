//
// Created by Gutnar Leede on 01/10/16.
//

#include "Detector.h"

using namespace std;
using namespace cv;

void Detector::onMouse(int event, int x, int y)
{
    if (event == EVENT_LBUTTONDOWN)
    {
        cout << x << " " << (IMAGE_HEIGHT - y) << endl;
    }
}

void Detector::mouseEventHandler(int event, int x, int y, int flags, void *userdata)
{
    Detector *detector = reinterpret_cast<Detector *>(userdata);
    detector->onMouse(event, x, y);
}

// Constructor
Detector::Detector(CSimpleIniA &configurationIni, CSimpleIniA &colorsIni) {
    //Pointers
    mConfigurationIni = &configurationIni;
    mColorsIni = &colorsIni;

    namedWindow("test");
    setMouseCallback("test", mouseEventHandler, this);
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

    // Smooth the image
    GaussianBlur(dstImage, dstImage, Size(25, 25), 2, 2);
}

vector<Detector::Ball> Detector::findBalls(Mat &srcImage) {
    Mat filteredImage;
    filterColor(srcImage, filteredImage, "ORANGE");

    /// BALL CONTAINER
    vector<Ball> balls;

    /// FIND CONTOURS
    vector<vector<Point> > contours;
    vector<Vec4i> hierarchy;

    Scalar red = Scalar(0, 0, 255);
    Scalar blue = Scalar(255, 0, 0);
    Scalar white = Scalar(255, 255, 255);

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
        int minX = IMAGE_WIDTH, maxX = 0;
        int maxY = 0;

        for (int j = 0; j < contourSize ; ++j) {
            sumX += contours[i][j].x;
            sumY += contours[i][j].y;

            minX = min(minX, contours[i][j].x);
            maxX = max(maxX, contours[i][j].x);
            maxY = max(maxY, contours[i][j].y);
        }

        ball.center = Point(sumX/contourSize, sumY/contourSize);

        drawContours(srcImage, contours, i, blue, 1);
        putText(srcImage, itos(IMAGE_HEIGHT - maxY), Point(ball.center.x + 50, ball.center.y), 1, 1, white);

        /**
         * s / y^2 = A
         * y = sqrt(s/A)
         */

        /// FIND THE BALLS' RADII
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

        /*
        if (ball.radius < 5 || ball.radius > 50) {
            continue;
        }

        if ((float) deviation/ball.radius > 0.5) {
            continue;
        }
         */

        //ball.radius = Q/contourSize;

        balls.push_back(ball);
    }

    imshow("test", srcImage);

    /// FIND CIRCLES THAT ARE NOT CONTAINED BY OTHER CIRCLES
    vector<Ball> singleBalls;

    for (int i = 0; i < balls.size(); ++i) {
        bool single = true;

        for (int j = 0; j < balls.size(); ++j) {
            if (i == j) {
                continue;
            }

            if (balls[i].radius > balls[j].radius) {
                continue;
            }

            int d = sqrt(pow(balls[i].center.x - balls[j].center.x, 2) + pow(balls[i].center.y - balls[j].center.y, 2));

            if (d < max(balls[i].radius, balls[j].radius)) {
                single = false;
                break;
            }
        }

        if (single) {
            singleBalls.push_back(balls[i]);
        }
    }

    return singleBalls;
}

/// GOAL
vector<vector<Point> > Detector::findGoal(Mat &srcImage, string color) {
    Mat filteredImage;
    filterColor(srcImage, filteredImage, color);

    // Display
    //namedWindow("filter");
    //imshow("filter", filteredImage);

    /// FIND CONTOURS
    vector<vector<Point> > contours;
    vector<Vec4i> hierarchy;

    // Find contours from filtered image
    findContours(filteredImage, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE, Point(0, 0));

    return contours;
}