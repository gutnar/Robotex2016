//
// Created by jk on 13.10.16.
//

#ifndef ROBOTEX2016_AI_H
#define ROBOTEX2016_AI_H


#include "Detector.h"

#define IMAGE_WIDTH 640
#define IMAGE_HALF_WIDTH 320
//#define IMAGE_HEIGHT 480

class AI {
public:
    AI();
    void notifyPositions(vector<Detector::Ball> &balls);
    string getCommand();
private:
    vector<Detector::Ball> mBalls;
    Detector::Ball* getClosestBall();

};


#endif //ROBOTEX2016_AI_H
