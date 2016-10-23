//
// Created by jk on 13.10.16.
//

#ifndef ROBOTEX2016_AI_H
#define ROBOTEX2016_AI_H


#include "Detector.h"
#include "common.h"

class AI
{
public:
    AI();

    void notify(vector<Detector::Ball> &balls, bool ballCaptured);

    string getCommand();

private:
    vector<Detector::Ball> mBalls;
    bool mBallCaptured;

    Detector::Ball *getClosestBall();

    // State
    enum
    {
        IDLE_STATE, CHOOSE_BALL_STATE, FIND_BALLS_STATE, GET_BALL_STATE, SHOOT_STATE, DRIBBLE_STATE
    };

    int mState = IDLE_STATE;

    // Target ball
    Detector::Ball *mTargetBall;
};


#endif //ROBOTEX2016_AI_H
