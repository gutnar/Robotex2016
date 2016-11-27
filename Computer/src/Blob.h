//
// Created by jk on 27.11.16.
//

#ifndef ROBOTEX2016_BLOB_H
#define ROBOTEX2016_BLOB_H

#include <vector>
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
    Blob();

    void addLine(BlobLine& line);
    void addBlob(Blob& blob);

    int mMinY = IMAGE_HEIGHT;
    int mMaxY = 0;
    int mMinX = IMAGE_WIDTH;
    int mMaxX = 0;

    int mSurface = 0;

    bool mHidden = false;

    std::vector<BlobLine> mLines;
};


#endif //ROBOTEX2016_BLOB_H
