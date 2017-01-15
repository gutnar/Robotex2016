//
// Created by jk on 13.10.16.
//

#ifndef ROBOTEX2016_AI_H
#define ROBOTEX2016_AI_H


#include "Detector.h"
#include "common.h"
#include "PID.h"

class AI {
public:
    AI();

    AI(Detector &detector);

    void notify(bool gameIsOn, vector<Detector::Ball> &balls, bool ballCaptured, Point opponentGoalCenter,
                Point ownGoalCenter, int opponentGoalWidth);

    string getCommand(int dt);

    string getSpeedCommand(float v, float angle);

private:
    vector<Detector::Ball> mBalls;
    bool mBallCaptured;
    bool mGameIsOn;

    Point mOwnGoalCenter;
    Point mOpponentGoalCenter;

    Detector *mDetector;

    Detector::Ball *getClosestBall();

    // State
    enum {
        IDLE_STATE,
        CHOOSE_BALL_STATE,
        FIND_BALLS_STATE,
        GET_BALL_STATE,
        SHOOT_STATE,
        DRIBBLE_STATE,
        FIND_GOAL_STATE,
        MOVE_TOWARDS_GOAL_STATE,
        FIND_BETTER_SHOOTING_ANGLE_STATE,
        BACK_UP_STATE,
        MOVE_SIDEWAYS_STATE
    };

    int mState = IDLE_STATE;

    // Target ball
    Detector::Ball mTargetBall;
    bool mTargetBallAvailable;

    // PID
    float mIntegral;
    float mPreviousError;

    PID mTurnPid = PID(3, 0.4, 0.4);
    PID mForwardPid = PID(1.5, 0.05, 0);
    PID mThirdWheelTurnPid;// = PID(0.2 , 0.01, 0.05);

    // Has the dribbler been stopped and has the kick command been sent during a shoot state
    bool mDribblerStopped;
    bool mKicked;

    // Line is crossed
    bool mLineIsCrossed;

    // How long has the dribbler been running
    int mTimer;

    // How long since no ball has been dribbled
    int mUselessTime;

    // How long has the aiming taken place
    int mAimTimer;

    // Where was the goal last seen
    bool mOpponentGoalIsLeft;
    bool mOpponentGoalWasLeft;

    //Where is the next closest ball last seen
    bool mNextClosestBallIsLeft;

    // How large the opponent goal is currently on screen in centimeters
    int mOpponentGoalWidth;

    // How many frames has the goal or target ball been in a good angle
    int mGoodAngleFrames;

    // Which goal to move towards when being useless
    bool mMoveTowardsOpponentGoal;
};


#endif //ROBOTEX2016_AI_H
