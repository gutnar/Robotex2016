//
// Created by jk on 27.11.16.
//

#ifndef ROBOTEX2016_BLOB_H
#define ROBOTEX2016_BLOB_H

#include <vector>
#include <opencv2/highgui.hpp>
#include "common.h"

struct BlobLine {
    int y;
    int xi;
    int xf;
    int color;
    int blobIndex;
};

class Blob {
public:
    Blob(int color);

    void addLine(BlobLine& line);
    void addBlob(Blob& blob);

    int mColor;

    int mMinY = IMAGE_HEIGHT;
    int mMaxY = 0;
    int mMinX = IMAGE_WIDTH;
    int mMaxX = 0;

    int mSumX = 0;
    int mSumY = 0;

    int mSurface = 0;

    bool mHidden = false;

    std::vector<BlobLine> mLines;

    int getWidth();
    int getHeight();
    int getSurface();
    cv::Point getCenter();
};


#endif //ROBOTEX2016_BLOB_H
