//
// Created by jk on 13.10.16.
//

#ifndef ROBOTEX2016_AI_H
#define ROBOTEX2016_AI_H


#include "Detector.h"
#include "common.h"

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
