//
// Created by jk on 27.11.16.
//

#include "Blob.h"

Blob::Blob(int color) {
    mColor = color;
}

void Blob::addLine(BlobLine &line) {
    if (line.xi < mMinX) mMinX = line.xi;
    if (line.xf > mMaxX) mMaxX = line.xf;
    if (line.y < mMinY) mMinY = line.y;
    if (line.y > mMaxY) mMaxY = line.y;

    mSumX += (line.xf - line.xi) * (line.xi + (line.xf - line.xi - 1)/2);
    mSumY += (line.xf - line.xi) * line.y;
    mSurface += line.xf - line.xi;

    //mLines.push_back(line);
}

void Blob::addBlob(Blob &blob) {
    if (blob.mMinX < mMinX) mMinX = blob.mMinX;
    if (blob.mMaxX > mMaxX) mMaxX = blob.mMaxX;
    if (blob.mMinY < mMinY) mMinY = blob.mMinY;
    if (blob.mMaxY > mMaxY) mMaxY = blob.mMaxY;

    mSumX += blob.mSumX;
    mSumY += blob.mSumY;
    mSurface += blob.mSurface;

    /*
    for (int i = 0; i < blob.mLines.size(); ++i) {
        mLines.push_back(blob.mLines[i]);
    }
     */
}

int Blob::getWidth() {
    return mMaxX - mMinX;
}

int Blob::getHeight() {
    return mMaxY - mMinY;
}

int Blob::getSurface() {
    return mSurface;
}

cv::Point Blob::getCenter() {
    return cv::Point(
            (mMaxX + mMinX) / 2,
            (mMaxY + mMinY) / 2
    );
}

cv::Point Blob::getMassCenter() {
    return cv::Point(
            mSumX/mSurface,
            mSumY/mSurface
    );
}