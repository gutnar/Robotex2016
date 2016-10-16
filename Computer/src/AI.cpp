//
// Created by jk on 13.10.16.
//

#include "AI.h"

AI::AI() {

}

void AI::notifyPositions(vector<Detector::Ball> &balls) {
    mBalls = balls;
}

Detector::Ball* AI::getClosestBall() {
    Detector::Ball *closestBall = NULL;

    for (int i = 0; i < mBalls.size(); ++i) {
        if (closestBall == NULL || mBalls[i].center.y > closestBall->center.y) {
            closestBall = &mBalls[i];
        }
    }

    return closestBall;
}

string AI::getCommand() {
    Detector::Ball *closestBall = getClosestBall();

    if (closestBall == NULL) {
        return "";
    }

    if (closestBall->center.x < IMAGE_HALF_WIDTH) {
        return "sd-10:-10:-10:0";
    }

    if (closestBall->center.x > IMAGE_HALF_WIDTH) {
        return "sd10:10:10:0";
    }

    return "";
}
