//
// Created by jk on 20.09.16.
//

#ifndef ROBOTEX2016_DETECTOR_H
#define ROBOTEX2016_DETECTOR_H

#include <vector>
#include <opencv2/opencv.hpp>

class Detector {
public:
    void calibrate(cv::VideoCapture cap);
    void onMouse(int event, int x, int y);
    static void mouseEventHandler(int event, int x, int y, int, void* userdata);
private:
    cv::Mat calibrationSrc;
    std::vector< std::vector<cv::Point> > polygons;
};


#endif //ROBOTEX2016_DETECTOR_H
