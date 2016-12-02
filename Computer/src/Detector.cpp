//
// Created by Gutnar Leede on 01/10/16.
//

#include "Detector.h"

void Detector::onMouse(int event, int x, int y) {
    if (event == EVENT_LBUTTONDOWN) {
        cout << abs(IMAGE_HALF_WIDTH - x) << " " << (IMAGE_HEIGHT - y) << endl;
    }
}

void Detector::mouseEventHandler(int event, int x, int y, int flags, void *userdata) {
    Detector *detector = reinterpret_cast<Detector *>(userdata);
    detector->onMouse(event, x, y);
}

// Constructor
Detector::Detector(CSimpleIniA &configurationIni, CSimpleIniA &colorsIni) {
    // Pointers
    mConfigurationIni = &configurationIni;
    mColorsIni = &colorsIni;

    // Color ranges
    string keys[6] = {"H_MIN", "H_MAX", "S_MIN", "S_MAX", "V_MIN", "V_MAX"};

    for (int i = 0; i < 6; ++i) {
        mWhite[i] = atoi(mColorsIni->GetValue("WHITE", keys[i].c_str()));
        mBlack[i] = atoi(mColorsIni->GetValue("BLACK", keys[i].c_str()));
    }

    //namedWindow("test");
    //setMouseCallback("test", mouseEventHandler, this);


    // Kernel
    vector<Platform> platforms;
    vector<Device> devices;

    // Create platform and get device
    Platform::get(&platforms);
    platforms[0].getDevices(CL_DEVICE_TYPE_GPU, &devices);
    mDevice = devices[0];

    // Create context
    Context context(devices);

    // load opencl source
    std::ifstream cl_file("src/kernels.cl");
    std::string cl_string(std::istreambuf_iterator<char>(cl_file),
                          (std::istreambuf_iterator<char>()));
    Program::Sources source(1, std::make_pair(cl_string.c_str(),
                                              cl_string.length() + 1));

    // create program and kernel and set kernel arguments
    mProgram = Program(context, source);

    if(mProgram.build(devices) != CL_SUCCESS){
        cout << "OpenCL program build failed!" << endl;
    }
};

void Detector::filterColor(Mat &srcImage, Mat &dstImage, string color) {
    // Load color data
    int values[6];
    string keys[6] = {"H_MIN", "H_MAX", "S_MIN", "S_MAX", "V_MIN", "V_MAX"};

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

    for (int i = 0; i < contours.size(); ++i) {
        //drawContours(srcImage, contours, i, blue, 2, 8, hierarchy, 0, Point());
        Ball ball;

        /// FIND THE CENTER OF CONTOURS
        int sumX = 0;
        int sumY = 0;
        int contourSize = (int) contours[i].size();
        int minX = IMAGE_WIDTH, maxX = 0;
        int maxY = 0;

        for (int j = 0; j < contourSize; ++j) {
            sumX += contours[i][j].x;
            sumY += contours[i][j].y;

            minX = min(minX, contours[i][j].x);
            maxX = max(maxX, contours[i][j].x);
            maxY = max(maxY, contours[i][j].y);
        }

        ball.center = Point(sumX / contourSize, sumY / contourSize);

        /// CHECK IF BALL IS WITHIN BORDERS
        /*
        if (!isBallWithinBorders(srcImage, ball)) {
            continue;
        }
         */

        // x-distance positive when ball on right half and negative when on left half
        ball.distance = FloatPoint(DISTANCE_C * (ball.center.x - IMAGE_HALF_WIDTH) / maxY,
                              DISTANCE_A + DISTANCE_B / maxY);

        float leftDistance = (DISTANCE_C * (minX - IMAGE_HALF_WIDTH) / maxY);
        float rightDistance = (DISTANCE_C * (maxX - IMAGE_HALF_WIDTH) / maxY);
        int width = round(rightDistance - leftDistance);


        if (width < 2 || width > 5) {
            continue;
        }

        drawContours(srcImage, contours, i, blue, 1);
        //putText(srcImage, itos(IMAGE_HEIGHT - maxY), Point(ball.center.x + 50, ball.center.y), 1, 1, white);
        putText(srcImage, itos(round(rightDistance - leftDistance)), Point(ball.center.x + 20, ball.center.y), 1, 1,
                white);

        balls.push_back(ball);
    }

    // center line for measuring x-axis distances
    line(srcImage, Point(IMAGE_HALF_WIDTH, 0), Point(IMAGE_HALF_WIDTH, IMAGE_HEIGHT), Scalar(255, 0, 255), 1);
    //imshow("test", srcImage);

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

    int largestContourIndex = -1;

    // Go through all contours
    for (int i = 0; i < contours.size(); ++i) {
        // Contour width
        int minX = IMAGE_WIDTH, maxX = 0;
        int minY = IMAGE_HEIGHT;

        for (int j = 0; j < contours[i].size(); ++j) {
            minX = min(minX, contours[i][j].x);
            maxX = max(maxX, contours[i][j].x);
            minY = min(minY, contours[i][j].y);
        }

        float leftDistance = (DISTANCE_C * (minX - IMAGE_HALF_WIDTH) / minY);
        float rightDistance = (DISTANCE_C * (maxX - IMAGE_HALF_WIDTH) / minY);
        int width = round(rightDistance - leftDistance);

        if (width < 60) {
            continue;
        }

        // Go through contour
        if (largestContourIndex == -1 || contours[i].size() > contours[largestContourIndex].size()) {
            largestContourIndex = i;
        }
    }

    if (largestContourIndex == -1) {
        return Point(0, 0);
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
    Point center((maxX - minX) / 2 + minX, (maxY - minY) / 2 + minY);
    //circle(filteredImage, center, 5, blue);

    //line(srcImage, Point())
    line(srcImage, Point(center.x, 0), Point(center.x, IMAGE_HEIGHT), Scalar(0, 255, 255), 1);
    imshow("goal", srcImage);

    // Test
    //drawContours(filteredImage, contours, largestContourIndex, blue, 1);

    // Display
    //namedWindow("test");

    return center;
}

bool Detector::isBallWithinBorders(int out[IMAGE_PIXELS], Detector::Ball ball) {
    int sequentialWhitePixels = 0;
    int sequentialBlackPixels = 0;
    int sequentialOtherPixels = 0;

    for (int y = IMAGE_HEIGHT - 1; y > ball.center.y; --y) {
        int color = out[y*IMAGE_WIDTH + ball.center.x];

        if (color == 0) { // white
            sequentialWhitePixels++;
            sequentialBlackPixels = 0;
            sequentialOtherPixels = 0;
        } else if (color == 2 || color == 3) { // black or blue
            sequentialBlackPixels++;
            sequentialOtherPixels = 0;
        } else {
            if (sequentialWhitePixels > 2 && sequentialBlackPixels > 2) {
                return false;
            }

            if (++sequentialOtherPixels > 6) {
                sequentialWhitePixels = 0;
                sequentialBlackPixels = 0;
            }
        }
    }

    return true;
}

int Detector::findBorder(int out[IMAGE_PIXELS], int x) {
    int sequentialWhitePixels = 0;
    int sequentialBlackPixels = 0;
    int sequentialOtherPixels = 0;

    for (int y = IMAGE_HEIGHT - 1; y >= 0; --y) {
        int color = out[y*IMAGE_WIDTH + x];

        if (color == 0) { // white
            sequentialWhitePixels++;
            sequentialBlackPixels = 0;
            sequentialOtherPixels = 0;
        } else if (color == 2 || color == 3) { // black or blue
            sequentialBlackPixels++;
            sequentialOtherPixels = 0;
        } else {
            if (sequentialWhitePixels > 2 && sequentialBlackPixels > 2) {
                return y + sequentialBlackPixels + sequentialWhitePixels + sequentialOtherPixels;
            }

            if (++sequentialOtherPixels > 6) {
                sequentialWhitePixels = 0;
                sequentialBlackPixels = 0;
            }
        }
    }

    return -1;
}

bool Detector::isPixelInColorRange(Vec3b pixel, int *color) {
    if (pixel[0] < color[0] || pixel[0] > color[1]) return false;
    if (pixel[1] < color[2] || pixel[1] > color[3]) return false;
    if (pixel[2] < color[4] || pixel[2] > color[5]) return false;

    return true;
}

void Detector::processImage(Mat& image) {
    // Convert BGR to HSV
    Mat workedImage;
    cvtColor(image, workedImage, COLOR_BGR2HSV);

    /// IMAGE DATA FOR OPENCL
    //std::vector<uint> array(workedImage.rows*workedImage.cols);
    //array = workedImage.data;
    //workedImage.col(0).copyTo(array);

    int in[IMAGE_PIXELS*3];

    //workedImage.data

    for (int y = 0; y < IMAGE_HEIGHT; ++y) {
        for (int x = 0; x < IMAGE_WIDTH; ++x) {
            Vec3b pixel = workedImage.at<Vec3b>(y, x);

            in[(y*IMAGE_WIDTH + x)*3 + 0] = pixel[0];
            in[(y*IMAGE_WIDTH + x)*3 + 1] = pixel[1];
            in[(y*IMAGE_WIDTH + x)*3 + 2] = pixel[2];
        }
    }

    //cout << (int) workedImage.data[0] << endl;

    int out[IMAGE_PIXELS];

    Buffer buffer_in(mContext, CL_MEM_READ_ONLY, sizeof(int)*IMAGE_PIXELS*3);
    Buffer buffer_out(mContext, CL_MEM_WRITE_ONLY, sizeof(int)*IMAGE_PIXELS);

    Kernel kernel(mProgram, "mark_pixels");
    kernel.setArg(0, buffer_in);
    kernel.setArg(1, buffer_out);

    /// MARK PIXELS
    CommandQueue queue(mContext, mDevice);
    queue.enqueueWriteBuffer(buffer_in, CL_TRUE, 0, sizeof(int)*IMAGE_PIXELS*3, in);
    queue.enqueueNDRangeKernel(kernel, NullRange, NDRange(IMAGE_WIDTH, IMAGE_HEIGHT), NullRange);
    queue.enqueueReadBuffer(buffer_out, CL_TRUE, 0, sizeof(int)*IMAGE_PIXELS, out);
    queue.finish();

    // Test
    for (int y = 0; y < IMAGE_HEIGHT; ++y) {
        for (int x = 0; x < IMAGE_WIDTH; ++x) {
            switch (out[y*IMAGE_WIDTH + x]) {
                // white
                case 0:
                    image.at<Vec3b>(y, x) = Vec3b(255, 255, 255);
                    //workedImage.at<Vec3b>(y, x) = colorMap["WHITE"];
                    break;
                    // yellow
                case 1:
                    image.at<Vec3b>(y, x) = Vec3b(0, 255, 255);
                    //workedImage.at<Vec3b>(y, x) = colorMap["ORANGE"];
                    break;
                    // blue
                case 2:
                    image.at<Vec3b>(y, x) = Vec3b(255, 0, 0);
                    //workedImage.at<Vec3b>(y, x) = colorMap["BLUE"];
                    break;
                    // black
                case 3:
                    image.at<Vec3b>(y, x) = Vec3b(0, 0, 0);
                    //workedImage.at<Vec3b>(y, x) = colorMap["BLACK"];
                    break;
                    // green
                case 4:
                    image.at<Vec3b>(y, x) = Vec3b(0, 255, 0);
                    //workedImage.at<Vec3b>(y, x) = colorMap["GREEN"];
                    break;
            }
        }
    }

    /// DETECT BLOBS OF SAME COLOR
    vector<Blob> blobs;
    vector<BlobLine> previousLines;

    for (int y = 0; y < IMAGE_HEIGHT; ++y) {
        vector<BlobLine> currentLines;

        int lastColor = out[y*IMAGE_WIDTH];
        int beginColorIndex = 0;

        for (int x = 1; x < IMAGE_WIDTH + 1; ++x) {
            int i = y*IMAGE_WIDTH + x;
            int currentColor = (x == IMAGE_WIDTH) ? -1 : out[i];

            if (lastColor != currentColor) {
                if ((lastColor == 1 || lastColor == 2) && x - 1 - beginColorIndex > 1) {//} || out[i] == 2) {
                    BlobLine line;
                    line.y = y;
                    line.xi = beginColorIndex;
                    line.xf = x - 1;
                    line.color = lastColor;
                    line.blobIndex = -1;

                    for (int l = 0; l < previousLines.size(); ++l) {
                        // Skip if colors do not match
                        if (previousLines[l].color != line.color) {
                            continue;
                        }

                        // Skip if lines do not touch
                        if (previousLines[l].xi > line.xf || previousLines[l].xf < line.xi) {
                            continue;
                        }

                        // Connect lines into blob
                        if (line.blobIndex == -1) {
                            line.blobIndex = previousLines[l].blobIndex;
                            //cout << line.blobIndex << endl;
                        } else if (line.blobIndex != previousLines[l].blobIndex) {
                            blobs[line.blobIndex].addBlob(blobs[previousLines[l].blobIndex]);
                            blobs[previousLines[l].blobIndex].mHidden = true;
                            previousLines[l].blobIndex = line.blobIndex;
                        }
                    }

                    // Create new blob?
                    if (line.blobIndex == -1) {
                        Blob blob(lastColor);
                        line.blobIndex = (int) blobs.size();
                        blobs.push_back(blob);
                    }

                    // Add line to blob
                    blobs[line.blobIndex].addLine(line);

                    // Add to current row
                    currentLines.push_back(line);
                }

                beginColorIndex = x;
                lastColor = currentColor;
            }
        }

        previousLines = currentLines;
    }

    // Test
    for (int i = 0; i < blobs.size(); ++i) {
        //image.at<Vec3b>(lines[i][0], lines[i][1]) = Vec3b(0, 0, 255);
        //image.at<Vec3b>(lines[i][0], lines[i][2]) = Vec3b(0, 0, 255);
        if (blobs[i].mMinY > IMAGE_HEIGHT/2 && blobs[i].mSurface < 50){
            blobs[i].mHidden = true;
        }

        if (!blobs[i].mHidden) {
            line(image, Point(blobs[i].mMinX, blobs[i].mMinY), Point(blobs[i].mMaxX, blobs[i].mMinY),
                 Scalar(0, 0, 255));
            line(image, Point(blobs[i].mMinX, blobs[i].mMaxY), Point(blobs[i].mMaxX, blobs[i].mMaxY),
                 Scalar(0, 0, 255));
            line(image, Point(blobs[i].mMinX, blobs[i].mMinY), Point(blobs[i].mMinX, blobs[i].mMaxY),
                 Scalar(0, 0, 255));
            line(image, Point(blobs[i].mMaxX, blobs[i].mMinY), Point(blobs[i].mMaxX, blobs[i].mMaxY),
                 Scalar(0, 0, 255));
        }
    }
}

FloatPoint Detector::getDistance(Point point) {
    if (point.y == 0) {
        return FloatPoint(10000, 10000);
    }

    return FloatPoint((float) DISTANCE_C * (point.x - IMAGE_HALF_WIDTH) / point.y, (float) (DISTANCE_A + DISTANCE_B / point.y));
}
