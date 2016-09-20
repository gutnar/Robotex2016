#include <opencv2/highgui.hpp>
#include "Detector.h"

using namespace cv;

int main() {
    VideoCapture cap;
    cap.open(1);

    Detector detector;
    detector.calibrate(cap);

    return 0;
}