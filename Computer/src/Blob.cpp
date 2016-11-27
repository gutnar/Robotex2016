//
// Created by jk on 27.11.16.
//

#include "Blob.h"

Blob::Blob() {
}

void Blob::addLine(BlobLine& line) {
    if (line.xi < mMinX) mMinX = line.xi;
    if (line.xf > mMaxX) mMaxX = line.xf;
    if (line.y < mMinY) mMinY = line.y;
    if (line.y > mMaxY) mMaxY = line.y;

    mLines.push_back(line);
}

void Blob::addBlob(Blob& blob) {
    if (blob.mMinX < mMinX) mMinX = blob.mMinX;
    if (blob.mMaxX > mMaxX) mMaxX = blob.mMaxX;
    if (blob.mMinY < mMinY) mMinY = blob.mMinY;
    if (blob.mMaxY > mMaxY) mMaxY = blob.mMaxY;

    for (int i = 0; i < blob.mLines.size(); ++i) {
        mLines.push_back(blob.mLines[i]);
    }
}
