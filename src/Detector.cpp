//
// Created by jk on 20.09.16.
//

#include "Detector.h"

using namespace std;
using namespace cv;

void Detector::onMouse(int event, int x, int y) {
    static bool drawing = false;

    if (event == cv::EVENT_LBUTTONDOWN) {
        std::vector<cv::Point> polygon;
        polygon.push_back(cv::Point(x, y));
        polygons.push_back(polygon);

        drawing = true;
    } else if (event == cv::EVENT_MOUSEMOVE) {
        if (drawing) {
            std::vector<cv::Point> *polygon = &polygons.back();
            cv::Point previousPoint = polygon->back();
            cv::Point currentPoint = cv::Point(x, y);

            if (cv::norm(currentPoint - previousPoint) >= 10) {
                polygon->push_back(currentPoint);
                line(calibrationSrc, previousPoint, currentPoint, cv::Scalar(255, 0, 255), 1);
                cv::imshow("Märgista värviga piirkond", calibrationSrc);
            }
        }
    } else if (event == cv::EVENT_LBUTTONUP) {
        std::vector<cv::Point> *polygon = &polygons.back();
        cv::line(calibrationSrc, polygon->back(), polygon->front(), cv::Scalar(255, 0, 255), 1);
        cv::imshow("Märgista värviga piirkond", calibrationSrc);

        drawing = false;
    }
}

void Detector::mouseEventHandler(int event, int x, int y, int flags, void *userdata) {
    Detector *detector = reinterpret_cast<Detector *>(userdata);
    detector->onMouse(event, x, y);
}

void Detector::calibrate(cv::VideoCapture cap) {
    // Get view
    cv::namedWindow("Aseta kalibreeritav värv kaamera vaatesse ja vajuta tühikut");

    while (1) {
        cap >> calibrationSrc;
        cv::imshow("Aseta kalibreeritav värv kaamera vaatesse ja vajuta tühikut", calibrationSrc);

        if (cv::waitKey(30) > 0) {
            break;
        }
    }

    cv::destroyWindow("Aseta kalibreeritav värv kaamera vaatesse ja vajuta tühikut");

    // Create HSV image
    cv::Mat hsvSrc;
    cv::cvtColor(calibrationSrc, hsvSrc, cv::COLOR_BGR2HSV);

    // Select areas
    cv::namedWindow("Märgista värviga piirkond");
    cv::imshow("Märgista värviga piirkond", calibrationSrc);
    cv::setMouseCallback("Märgista värviga piirkond", mouseEventHandler, this);

    while (1) {
        if (cv::waitKey(30) > 0) {
            break;
        }
    }

    // Calibrate
    int pixels = 0;
    std::vector<int> HSV[3];

    for (int i = 0; i < calibrationSrc.rows; ++i) {
        // Get polygon points on this row
        std::vector<int> nodes;

        for (int j = 0; j < calibrationSrc.cols; ++j) {
            cv::Vec3b bgrPixel = calibrationSrc.at<cv::Vec3b>(i, j);

            if (bgrPixel[0] == 255 && bgrPixel[1] == 0 && bgrPixel[2] == 255) {
                nodes.push_back(j);
            }
        }

        // Select points between polygon node pairs
        for (int j = 0; j < nodes.size() - nodes.size() % 2; j += 2) {
            for (int k = nodes[j]; k < nodes[j + 1]; ++k) {
                calibrationSrc.at<cv::Vec3b>(i, k) = cv::Vec3b(255, 0, 255);
                cv::Vec3b hsvPixel = hsvSrc.at<cv::Vec3b>(i, k);

                HSV[0].push_back((int) hsvPixel[0]);
                HSV[1].push_back((int) hsvPixel[1]);
                HSV[2].push_back((int) hsvPixel[2]);

                ++pixels;
            }
        }
    }

    cv::imshow("Märgista värviga piirkond", calibrationSrc);

    // Calculate means and deviations
    int range[3][2];

    for (int i = 0; i < 3; ++i) {
        int sum = 0;

        for (int j = 0; j < pixels; ++j) {
            sum += HSV[i][j];
        }

        int mean = sum / pixels;
        int deviationSquareSum = 0;

        for (int j = 0; j < pixels; ++j) {
            deviationSquareSum += pow(HSV[i][j] - mean, 2);
        }

        int deviation = (int) sqrt(deviationSquareSum / (pixels - 1));

        range[i][0] = mean - deviation * 1.5;
        range[i][1] = mean + deviation * 1.5;
    }

    std::cout << "H " << range[0][0] << " - " << range[0][1] << std::endl;
    std::cout << "S " << range[1][0] << " - " << range[1][1] << std::endl;
    std::cout << "V " << range[2][0] << " - " << range[2][1] << std::endl;

    // Close calibration window
    destroyWindow("Märgista värviga piirkond");

    // Show results
    Mat image;
    Mat workedImage;

    namedWindow("Tulemused");

    while (1) {
        cap >> image;

        cv::cvtColor(image, workedImage, cv::COLOR_BGR2HSV);
        cv::inRange(workedImage, cv::Scalar(range[0][0], range[1][0], 0), cv::Scalar(range[0][1], range[1][1], 255), workedImage);

        // Smooth it, otherwise a lot of false circles may be detected
        cv::GaussianBlur(workedImage, workedImage, cv::Size(25, 25), 2, 2);

        // Erode
        cv::erode(workedImage, workedImage, cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(10, 10), cv::Point(0, 0)));

        // Dilate
        //cv::dilate(image, image, cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(10, 10), cv::Point(0, 0)));

        /// Find contours
        std::vector<std::vector<cv::Point> > contours;
        std::vector<cv::Vec4i> hierarchy;

        findContours(workedImage, contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE, cv::Point(0, 0));

        /// Draw contours
        //cv::Mat drawing = cv::Mat::zeros(image.size(), CV_8UC3);

        for (int i = 0; i < contours.size(); i++)
        {
            Scalar color = Scalar(255, 0, 0);
            drawContours(image, contours, i, color, 2, 8, hierarchy, 0, Point());
        }

        // Show detected areas
        imshow("Tulemused", image);

        if (waitKey(30) > 0) {
            break;
        }
    }
}