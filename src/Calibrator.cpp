//
// Created by jk on 20.09.16.
//

#include "Calibrator.h"

using namespace std;
using namespace cv;

void Calibrator::onMouse(int event, int x, int y)
{
    static bool drawing = false;

    if (event == EVENT_LBUTTONDOWN)
    {
        vector<Point> polygon;
        polygon.push_back(Point(x, y));
        polygons.push_back(polygon);

        drawing = true;
    } else if (event == EVENT_MOUSEMOVE)
    {
        if (drawing)
        {
            vector<Point> *polygon = &polygons.back();
            Point previousPoint = polygon->back();
            Point currentPoint = Point(x, y);

            if (norm(currentPoint - previousPoint) >= 10)
            {
                polygon->push_back(currentPoint);
                line(calibrationSrc, previousPoint, currentPoint, Scalar(255, 0, 255), 1);
                imshow(windowName, calibrationSrc);
            }
        }
    } else if (event == EVENT_LBUTTONUP)
    {
        vector<Point> *polygon = &polygons.back();
        line(calibrationSrc, polygon->back(), polygon->front(), Scalar(255, 0, 255), 1);
        imshow(windowName, calibrationSrc);

        drawing = false;
    }
}

void Calibrator::mouseEventHandler(int event, int x, int y, int flags, void *userdata)
{
    Calibrator *detector = reinterpret_cast<Calibrator *>(userdata);
    detector->onMouse(event, x, y);
}

int isCircular(vector<Point> contour)
{
    /*
     *
     * V = \sum_i [ R - |x_i - r_x| ]^2
     * dV/dR = \sum_i 2 [ R - |x_i - r_x| ] = 0
     * dV/dr_x = \sum_i 2 [ R - |x_i - r_x| ] = 0
     *
     */

    int i;
    vector<float> curvatures;

    int n = contour.size();
    float sum, sumSq;

    if (n < 50)
    {
        return 0;
    }

    float k1 = abs((float) (contour[0 + 5].y - contour[0].y) / (contour[0 + 5].x - contour[0].x));

    for (i = 5; i < contour.size() - 5; i += 5)
    {
        float k2 = abs((float) (contour[i + 5].y - contour[i].y) / (contour[i + 5].x - contour[i].x));
        float localCurvature = k2 - k1;
        k1 = k2;

        sum += localCurvature;
        sumSq += localCurvature * localCurvature;
    }

    float mean = sum / n;
    float deviation = sqrt(sumSq - mean * mean);

    cout << mean << " +- " << deviation << endl;

    if (deviation / mean < 0.25)
    {
        return 1;
    } else
    {
        return 0;
    }
}

void Calibrator::calibrate(VideoCapture cap, CSimpleIniA *ini) {
    string colors[2] = {"ORANGE", "BLUE"};

    for (int i = 0; i < 2; ++i) {
        calibrateColor(cap, ini, colors[i]);
    }
}

void Calibrator::calibrateColor(VideoCapture cap, CSimpleIniA *ini, string color)
{
    // Create window
    if (color == "ORANGE")
    {
        windowName = "Calibrate orange";
    } else if (color == "BLUE")
    {
        windowName = "Calibrate blue";
    }

    // Get view
    namedWindow(windowName);

    while (1)
    {
        cap >> calibrationSrc;
        imshow(windowName, calibrationSrc);

        if (waitKey(30) > 0)
        {
            break;
        }
    }

    // Create HSV image
    Mat hsvSrc;
    cvtColor(calibrationSrc, hsvSrc, COLOR_BGR2HSV);

    // Select areas
    setMouseCallback(windowName, mouseEventHandler, this);

    while (1)
    {
        if (waitKey(30) > 0)
        {
            break;
        }
    }

    // Calibrate
    int pixels = 0;
    vector<int> HSV[3];

    for (int i = 0; i < calibrationSrc.rows; ++i)
    {
        // Get polygon points on this row
        vector<int> nodes;

        for (int j = 0; j < calibrationSrc.cols; ++j)
        {
            // Get BGR color on image at row i and column j
            Vec3b bgrPixel = calibrationSrc.at<Vec3b>(i, j);

            if (bgrPixel[0] == 255 && bgrPixel[1] == 0 && bgrPixel[2] == 255)
            {
                if (nodes.size() && nodes.back() == j - 1)
                {
                    nodes[nodes.size() - 1] = j;
                } else
                {
                    nodes.push_back(j);
                }
            }
        }

        // Select points between polygon node pairs
        for (int j = 0; j < nodes.size() - nodes.size() % 2; j += 2)
        {
            for (int k = nodes[j]; k < nodes[j + 1]; ++k)
            {
                // Make pixel on BGR image purple
                calibrationSrc.at<Vec3b>(i, k) = Vec3b(255, 0, 255);

                // Get pixel color on HSV image
                Vec3b hsvPixel = hsvSrc.at<Vec3b>(i, k);

                HSV[0].push_back((int) hsvPixel[0]); // hue value
                HSV[1].push_back((int) hsvPixel[1]); // saturation value
                HSV[2].push_back((int) hsvPixel[2]); // value value

                ++pixels;
            }
        }
    }

    imshow(windowName, calibrationSrc);

    // Calculate means and deviations
    int range[3][2]; // quasirow, -column

    for (int i = 0; i < 3; ++i)
    {
        int sum = 0;
        int sqSum = 0;

        for (int j = 0; j < pixels; ++j)
        {
            sum += HSV[i][j];
            sqSum += pow(HSV[i][j], 2);
        }

        int mean = sum / pixels;
        int deviation = (int) sqrt(sqSum / pixels - mean * mean);

        // Set min and max HSV values to filter on image
        range[i][0] = mean - deviation;
        range[i][1] = mean + deviation;
    }

    cout << "H " << range[0][0] << " - " << range[0][1] << endl;
    cout << "S " << range[1][0] << " - " << range[1][1] << endl;
    cout << "V " << range[2][0] << " - " << range[2][1] << endl;

    // SHOW RESULTS
    Mat image; //pixel container type - Mat
    Mat workedImage;

    while (1)
    {
        cap >> image;

        // Convert BGR to HSV
        cvtColor(image, workedImage, COLOR_BGR2HSV);
        inRange(workedImage, Scalar(range[0][0], range[1][0], 0), Scalar(range[0][1], range[1][1], 255),
                workedImage);

        // Smooth it, otherwise a lot of false circles may be detected
        GaussianBlur(workedImage, workedImage, Size(25, 25), 2, 2);

        // Erode
        erode(workedImage, workedImage,
              getStructuringElement(MORPH_ELLIPSE, Size(10, 10), Point(0, 0)));

        // Dilate
        //cv::dilate(image, image, cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(10, 10), cv::Point(0, 0)));

        /// FIND CONTOURS
        vector<vector<Point> > contours;
        vector<Vec4i> hierarchy;

        // Find contours from filtered image (workedImage)
        findContours(workedImage, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE, Point(0, 0));

        Scalar blue = Scalar(255, 0, 0);
        Scalar green = Scalar(0, 255, 0);
        Scalar red = Scalar(0, 0, 255);

        for (int i = 0; i < contours.size(); ++i)
        {
            drawContours(image, contours, i, blue, 2, 8, hierarchy, 0, Point());
        }

        // Show detected areas
        imshow(windowName, image);

        if (waitKey(30) > 0)
        {
            break;
        }
    }

    destroyWindow(windowName);

    // SAVE

    ini->SetValue(color.c_str(), "H_MIN", to_string(range[0][0]).c_str());
    ini->SetValue(color.c_str(), "H_MAX", to_string(range[0][1]).c_str());
    ini->SetValue(color.c_str(), "S_MIN", to_string(range[1][0]).c_str());
    ini->SetValue(color.c_str(), "S_MAX", to_string(range[1][1]).c_str());
    ini->SetValue(color.c_str(), "V_MIN", to_string(range[2][0]).c_str());
    ini->SetValue(color.c_str(), "V_MAX", to_string(range[2][1]).c_str());


    /*
    string save;

    cout << "Save (y/n)? ";
    cin >> save;

    if (save == "y") {
        string place, color;

        cout << "Place: ";
        cin >> place;
        cout << "Color (G/W/R/B/Y): ";
        cin >> color;

        ofstream file;

        file.open ("calibration.txt");
        file << "PLACE " << place << "\n";
        file << "COLOR " << color << "\n";
        file << "H " << range[0][0] << " " << range[0][1] << "\n";
        file << "S " << range[1][0] << "-" << range[1][1] << "\n";
        file << "V " << range[2][0] << "-" << range[2][1] << "\n";

        file.close();
    }
     */
}