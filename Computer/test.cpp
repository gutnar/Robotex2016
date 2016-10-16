#include "opencv2/opencv.hpp"
#include "opencv2/highgui/highgui.hpp"

using namespace cv;
using namespace std;

Mat src, hsv;
vector<vector<Point>> polygons;

int pointInPolygon(float x, float y)
{
    for (int k = 0; k < polygons.size(); ++k)
    {
        int c = 0;
        vector<Point> p = polygons[k];
        int n = (int) p.size();

        for (int i = 0, j = n - 1; i < n; j = i++)
        {
            if (((p[i].y > y) != (p[j].y > y)) &&
                    (x < (p[j].x - p[i].x) * (y - p[i].y) / (p[j].y - p[i].y) + p[i].x))
            {
                c = !c;
            }
        }

        if (c)
        {
            return 1;
        }
    }

    return 0;
}

void mouseEventHandler(int event, int x, int y, int flags, void* userdata)
{
    static bool drawing = false;

    if (event == EVENT_LBUTTONDOWN)
    {
        vector<Point> polygon;
        polygon.push_back(Point(x, y));
        polygons.push_back(polygon);

        drawing = true;
    }
    else if (event == EVENT_MOUSEMOVE)
    {
        if (drawing)
        {
            vector<Point>* polygon = &polygons.back();

            Point previousPoint = polygon->back();
            Point currentPoint = Point(x, y);

            if (norm(currentPoint - previousPoint) >= 20)
            {
                polygon->push_back(currentPoint);
                line(src, previousPoint, currentPoint, Scalar(255, 0, 255), 1);
                imshow("raw", src);
            }
        }
    }
    else if (event == EVENT_LBUTTONUP)
    {
        vector<Point>* polygon = &polygons.back();

        line(src, polygon->back(), polygon->front(), Scalar(255, 0, 255), 1);
        imshow("raw", src);

        drawing = false;
    }
}

int main(int, char**)
{
    VideoCapture cap(0); // open the default camera
    if (!cap.isOpened())  // check if we succeeded
        return -1;

    // Window
    namedWindow("raw");
    //namedWindow("threshed_image", 1);

    // HSV trackbars
    int minHSV[3] = {0, 0, 0};
    int maxHSV[3] = {179, 255, 255};

    createTrackbar("Minimum hue", "threshed_image", &minHSV[0], 179);
    createTrackbar("Maximum hue", "threshed_image", &maxHSV[0], 179);

    createTrackbar("Minimum saturation", "threshed_image", &minHSV[1], 255);
    createTrackbar("Maximum saturation", "threshed_image", &maxHSV[1], 255);

    createTrackbar("Minimum value", "threshed_image", &minHSV[2], 255);
    createTrackbar("Maximum value", "threshed_image", &maxHSV[2], 255);

    // Select frame
    Mat frame, canny, edges;
    RNG rng(12345);

    for (;;)
    {
        cap >> src; // get a new frame from camera
        cvtColor(src, hsv, COLOR_BGR2HSV);

        /*
        cvtColor(frame, frame, COLOR_BGR2GRAY);
        GaussianBlur(frame, frame, Size(11, 11), 10, 10);
        Canny(frame, canny, 15, 30, 3);

        // Find contours
        vector<vector<Point>> contours;
        vector<Vec4i> hierarchy;

        findContours(canny, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));

        for(int i = 0; i< contours.size(); i++)
        {
            Scalar color = Scalar(rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255));
            drawContours(frame, contours, i, Scalar(255, 255, 255));
        }

        //Mat dst;
        //dst = Scalar::all(0);
        //frame.copyTo(dst, canny);
        //cvtColor(canny, canny, CV_GRAY2BGR); // convert canny image to bgr
        //frame += canny; // add src image with canny image

        //Mat threshed_image;
        //inRange(hsv, Scalar(minHSV[0], minHSV[1], minHSV[2]), Scalar(maxHSV[0], maxHSV[1], maxHSV[2]), threshed_image);
         */

        imshow("raw", src);
        //imshow("threshed_image", threshed_image);

        if (waitKey(30) >= 0) break;
    }

    // Draw areas to detect
    for (;;)
    {
        setMouseCallback("raw", mouseEventHandler, NULL);

        if(waitKey(30) >= 0) break;
    }

    // Calibrate
    int pixels = 0;
    bool inside = false;
    bool alongLine = false;

    Vector<int> HSV[3];

    for(int i = 0; i < src.rows; ++i)
    {
        // Get polygon points on this row
        Vector<int> nodes;

        for (int j = 0; j < src.cols; ++j)
        {
            Vec3b bgrPixel = src.at<Vec3b>(i, j);

            if (bgrPixel[0] == 255 && bgrPixel[1] == 0 && bgrPixel[2] == 255)
            {
                nodes.push_back(j);
            }
        }

        // Select points between polygon node pairs
        for (int j = 0; j < nodes.size() - nodes.size() % 2; j += 2)
        {
            cout << nodes.size() << ": " << nodes[j] << " (" << j << ") to " << nodes[j+1] << " (" << (j+1) << ")" << endl;

            for (int k = nodes[j]; k < nodes[j+1]; ++k)
            {
                src.at<Vec3b>(i, k) = Vec3b(255, 0, 255);

                Vec3b hsvPixel = hsv.at<Vec3b>(i, j);

                HSV[0].push_back(hsvPixel[0]);
                HSV[1].push_back(hsvPixel[1]);
                HSV[2].push_back(hsvPixel[2]);

                ++pixels;
            }
        }

        /*
        for(int j = 0; j < src.cols; ++j)
        {
            Vec3b &bgrPixel = src.at<Vec3b>(i, j);

            if (bgrPixel[0] == 255 && bgrPixel[1] == 0 && bgrPixel[2] == 255)
            {
                if (!alongLine)
                {
                    inside = !inside;
                    alongLine = true;
                }
            }
            else if (inside) {
            //if (pointInPolygon(i, j))
           // {
                cout << i << "x" << j << endl;

                Vec3b hsvPixel = hsv.at<Vec3b>(i, j);

                HSV[0].push_back(hsvPixel[0]);
                HSV[1].push_back(hsvPixel[1]);
                HSV[2].push_back(hsvPixel[2]);

                bgrPixel[0] = 255;
                bgrPixel[1] = 0;
                bgrPixel[2] = 255;

                ++pixels;
            //}

                alongLine = false;
            }
        }
         */
    }

    imshow("raw", src);

    // Calculate means and deviations
    int range[3][2];

    for (int i = 0; i < 3; ++i)
    {
        int sum = 0;

        for (int j = 0; j < pixels; ++j)
        {
            sum += HSV[i][j];
        }

        int mean = sum / pixels;
        int deviationSquareSum = 0;

        for (int j = 0; j < pixels; ++j)
        {
            deviationSquareSum += pow(HSV[i][j] - mean, 2);
        }

        int deviation = sqrt(deviationSquareSum/(pixels - 1));

        range[i][0] = mean - deviation;
        range[i][1] = mean + deviation;

        cout << mean << " +/- " << deviation << endl;
    }

    inRange(hsv, Scalar(range[0][0], range[1][0], range[2][0]), Scalar(range[0][1], range[1][1], range[2][1]), hsv);

    // Show results
    namedWindow("results");
    imshow("results", hsv);

    // Wait
    for (;;)
    {
        if(waitKey(30) >= 0) break;
    }

    // the camera will be deinitialized automatically in VideoCapture destructor
    return 0;
}
