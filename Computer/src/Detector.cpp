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
        cout << abs(IMAGE_HALF_WIDTH - x) << " " << (IMAGE_HEIGHT - y) << endl;
    }
}

void Detector::mouseEventHandler(int event, int x, int y, int flags, void *userdata)
{
    Detector *detector = reinterpret_cast<Detector *>(userdata);
    detector->onMouse(event, x, y);
}

// Constructor
Detector::Detector(CSimpleIniA &configurationIni, CSimpleIniA &colorsIni) {
    // Pointers
    mConfigurationIni = &configurationIni;
    mColorsIni = &colorsIni;

    // Color ranges
    string keys[6] = {"H_MIN","H_MAX","S_MIN","S_MAX","V_MIN","V_MAX"};

    for (int i = 0; i < 6; ++i)
    {
        mWhite[i] = atoi(mColorsIni->GetValue("WHITE", keys[i].c_str(), NULL));
        mBlack[i] = atoi(mColorsIni->GetValue("BLACK", keys[i].c_str(), NULL));
    }

    //namedWindow("test");
    //setMouseCallback("test", mouseEventHandler, this);
};

void Detector::filterColor(Mat &srcImage, Mat &dstImage, string color) {
    // Load color data
    int values[6];
    string keys[6] = {"H_MIN","H_MAX","S_MIN","S_MAX","V_MIN","V_MAX"};

    for (int i = 0; i < 6; ++i) {
        values[i] = atoi(mColorsIni->GetValue(color.c_str(), keys[i].c_str(), NULL));
    }

    // Filters workedImage according to a color
    //inRange(srcImage, Scalar(values[0], values[2], 0), Scalar(values[1], values[3], 255), dstImage);
    inRange(srcImage, Scalar(values[0], values[2], values[4]), Scalar(values[1], values[3], values[5]), dstImage);

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

        /// CHECK IF BALL IS WITHIN BORDERS
        if (!isBallWithinBorders(srcImage, ball)) {
            continue;
        }

        // x-distance positive when ball on right half and negative when on left half
        ball.distance = Point(round(DISTANCE_C*(ball.center.x - IMAGE_HALF_WIDTH)/maxY), round(DISTANCE_A + DISTANCE_B/maxY));

        float leftDistance = (DISTANCE_C*(minX - IMAGE_HALF_WIDTH)/maxY);
        float rightDistance = (DISTANCE_C*(maxX - IMAGE_HALF_WIDTH)/maxY);
        float width = rightDistance - leftDistance;

        if (width < 3 || width > 5) {
            continue;
        }

        drawContours(srcImage, contours, i, blue, 1);
        //putText(srcImage, itos(IMAGE_HEIGHT - maxY), Point(ball.center.x + 50, ball.center.y), 1, 1, white);
        putText(srcImage, itos(round(rightDistance-leftDistance)), Point(ball.center.x+20, ball.center.y), 1, 1, white);

        balls.push_back(ball);
    }

    // center line for measuring x-axis distances
    line(srcImage, Point(IMAGE_HALF_WIDTH, 0), Point(IMAGE_HALF_WIDTH, IMAGE_HEIGHT), Scalar(255, 0, 255), 1);
    imshow("test", srcImage);

    return balls;
}

/// GOAL
Point Detector::findGoal(Mat &srcImage, string color) {
    Mat filteredImage;
    filterColor(srcImage, filteredImage, color);

    /// FIND CONTOURS
    vector<vector<Point> > contours;
    vector<Vec4i> hierarchy;

    // Find contours from filtered image
    findContours(filteredImage, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE, Point(0, 0));

    Scalar blue = Scalar(255, 0, 0);

    if (contours.size()) {
        int largestContourIndex = 0;
        // Go through all contours
        for (int i = 0; i < contours.size(); ++i) {
            // Go through contour
            if (contours[i].size() > contours[largestContourIndex].size()) {
                largestContourIndex = i;
            }

        }

        vector<Point> largestContour = contours[largestContourIndex];

        int minY = IMAGE_WIDTH;
        int maxY = 0;
        int minX = IMAGE_HEIGHT;
        int maxX = 0;

        // The center of the contour
        for (int j = 0; j < largestContour.size(); ++j) {
            if (minY > largestContour[j].y) minY = largestContour[j].y;
            if (maxY < largestContour[j].y) maxY = largestContour[j].y;
            if (minX > largestContour[j].x) minX = largestContour[j].x;
            if (maxX < largestContour[j].x) maxX = largestContour[j].x;

            //drawContours(filteredImage, contours, i, blue, 1);
        }

        // Center  coordinates
        Point center((maxX-minX)/2 + minX, (maxY-minY)/2 + minY);
        //circle(filteredImage, center, 5, blue);

        //line(srcImage, Point())
        line(srcImage, Point(center.x, 0), Point(center.x, IMAGE_HEIGHT), Scalar(0, 255, 255), 1);
        
        // Test
        //drawContours(filteredImage, contours, largestContourIndex, blue, 1);

        // Display
        //namedWindow("test");

        return center;
    }

    return Point(0, 0);
}

bool Detector::isBallWithinBorders(Mat &srcImage, Detector::Ball ball)
{
    //Mat filteredImage;
    //filterColor(srcImage, filteredImage, "BLACK");
    //imshow("detector", srcImage);

    //cout << "hello" << endl;

    //for (int x = 0; x < srcImage.cols; ++x) {
    int sequentialWhitePixels = 0;
    int sequentialBlackPixels = 0;
    int sequentialOtherPixels = 0;

    for (int y = srcImage.rows - 1; y > ball.center.y; --y) {
        Vec3b pixel = srcImage.at<Vec3b>(y, ball.center.x);

        //cout << pixel[0] << " " << pixel[1] << " " << pixel[2] << endl;

        if (isPixelInColorRange(pixel, mWhite)) {
            sequentialWhitePixels++;
            sequentialBlackPixels = 0;
            sequentialOtherPixels = 0;
        } else if (isPixelInColorRange(pixel, mBlack)) {
            sequentialBlackPixels++;
            sequentialOtherPixels = 0;
        } else {
            // TODO: ball on border
            //srcImage.at<Vec3b>(y, ball.center.x) = Vec3b(255, 0, 255);

            if (sequentialWhitePixels > 2 && sequentialBlackPixels > 2)
            {
                return false;
                cout << sequentialWhitePixels << " " << sequentialBlackPixels << endl;
                break;
            }

            if (++sequentialOtherPixels > 2)
            {
                sequentialWhitePixels = 0;
                sequentialWhitePixels = 0;
            }
        }
    }
    //}

    //imshow("detector", srcImage);

    return true;
}

int Detector::findBorder(Mat &srcImage, int x)
{
    //Mat filteredImage;
    //filterColor(srcImage, filteredImage, "BLACK");
    //imshow("detector", srcImage);

    //cout << "hello" << endl;

    //for (int x = 0; x < srcImage.cols; ++x) {
    int sequentialWhitePixels = 0;
    int sequentialBlackPixels = 0;
    int sequentialOtherPixels = 0;

    for (int y = srcImage.rows - 1; y >= 0; --y) {
        Vec3b pixel = srcImage.at<Vec3b>(y, x);

        //cout << pixel[0] << " " << pixel[1] << " " << pixel[2] << endl;

        if (isPixelInColorRange(pixel, mWhite)) {
            sequentialWhitePixels++;
            sequentialBlackPixels = 0;
            sequentialOtherPixels = 0;
        } else if (isPixelInColorRange(pixel, mBlack)) {
            sequentialBlackPixels++;
            sequentialOtherPixels = 0;
        } else {
            // TODO: ball on border
            //srcImage.at<Vec3b>(y, ball.center.x) = Vec3b(255, 0, 255);

            if (sequentialWhitePixels > 2 && sequentialBlackPixels > 2)
            {
                return y;
                cout << sequentialWhitePixels << " " << sequentialBlackPixels << endl;
                break;
            }

            if (++sequentialOtherPixels > 2)
            {
                sequentialWhitePixels = 0;
                sequentialWhitePixels = 0;
            }
        }
    }
    //}

    //imshow("detector", srcImage);

    return -1;
}

bool Detector::isPixelInColorRange(Vec3b pixel, int *color)
{
    if (pixel[0] < color[0] || pixel[0] > color[1]) return false;
    if (pixel[1] < color[2] || pixel[1] > color[3]) return false;
    if (pixel[2] < color[4] || pixel[2] > color[5]) return false;

    return true;
}

