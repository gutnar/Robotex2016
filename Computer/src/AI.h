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

    void notify(bool gameIsOn, vector<Detector::Ball> &balls, bool ballCaptured, Point goalCenter);

    string getCommand(int dt);

private:
    vector<Detector::Ball> mBalls;
    bool mBallCaptured;
    bool mGameIsOn;
    Point mGoalCenter;

    Detector::Ball *getClosestBall();

    // State
    enum
    {
        IDLE_STATE, CHOOSE_BALL_STATE, FIND_BALLS_STATE, GET_BALL_STATE, SHOOT_STATE, DRIBBLE_STATE, FIND_GOAL_STATE, MOVE_TOWARDS_GOAL_STATE
    };

    int mState = IDLE_STATE;

    // Target ball
    Detector::Ball *mTargetBall;

    // PID
    float mIntegral;
    float mPreviousError;

    // Has the dribbler been stopped and has the kick command been sent during a shoot state
    bool mDribblerStopped;
    bool mKicked;

    // How long has the dribbler been running
    int mDribblerRuntime;

    // Where was the goal last seen
    bool mGoalWasLeft;
};


#endif //ROBOTEX2016_AI_H
