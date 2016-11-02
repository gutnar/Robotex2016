//
// Created by jk on 13.10.16.
//

#include "AI.h"

AI::AI()
{

}

void AI::notify(bool gameIsOn, vector<Detector::Ball> &balls, bool ballCaptured, Point goalCenter)
{
    mGameIsOn = gameIsOn;
    mBalls = balls;
    mBallCaptured = ballCaptured;
    mGoalCenter = goalCenter;
}

Detector::Ball *AI::getClosestBall()
{
    Detector::Ball *closestBall = NULL;

    for (int i = 0; i < mBalls.size(); ++i)
    {
        if (closestBall == NULL || mBalls[i].center.y > closestBall->center.y)
        {
            closestBall = &mBalls[i];
        }
    }

    return closestBall;
}

string AI::getCommand()
{
    if (!mGameIsOn) {
        mState = IDLE_STATE;
    }

    switch (mState)
    {
        case IDLE_STATE:
            if (mGameIsOn) {
                mState = CHOOSE_BALL_STATE;
            } else {
                return "sd0:0:0:0\nd0";
            }
            break;
        case CHOOSE_BALL_STATE:
            mTargetBall = getClosestBall();

            if (mTargetBall == NULL)
            {
                mState = FIND_BALLS_STATE;
            } else
            {
                mState = GET_BALL_STATE;
            }
            break;
        case FIND_BALLS_STATE:
            if (mBalls.size()) {
                mState = GET_BALL_STATE;
            } else {
                return "sd10:10:10:0";
            }
            break;
        case SHOOT_STATE:
            // FIND GOAL
            if (mGoalCenter.x == 0 && mGoalCenter.y == 0) {
                return "sd7:7:7:0";
            } else {
                int difference = 5;
                if (mGoalCenter.x > IMAGE_HALF_WIDTH + difference)
                {
                    return "sd5:5:5:0";
                } else if (mGoalCenter.x < IMAGE_HALF_WIDTH - difference) {
                    return "sd-5:-5:-5:0";
                } else {
                    return "sd0:0:0:0";
                }
            }
            break;
        case GET_BALL_STATE:
            //cout << mTargetBall->center.x << endl;
            mTargetBall = getClosestBall();

            if (mTargetBall == NULL) {
                mState = FIND_BALLS_STATE;
            } else {
                double angle = atan((double) mTargetBall->distance.x / (mTargetBall->distance.y + 20)) * 180 / 3.14159;

                //cout << angle << endl;

                if (abs(mTargetBall->distance.x) < 4) {
                    if (mTargetBall->distance.y < 10) {
                        mState = DRIBBLE_STATE;
                    } else if (mTargetBall->distance.y < 25) {
                        return "sd-25:25:0:0";
                    } else {
                        if (mTargetBall->distance.x < -2) {
                            return "sd-25:25:5:0";
                        } else if (mTargetBall->distance.x > 2) {
                            return "sd-25:25:-5:0";
                        } else {
                            return "sd-25:25:0:0";
                        }
                        //return "sd0:0:0:0";
                    }

                }

                int turnSpeed = 6;// min((int) abs(angle), 10);

                if (angle < 0) {
                    return "sd-" + itos(turnSpeed) + ":-" + itos(turnSpeed) + ":-" + itos(turnSpeed) + ":0";
                }

                if (angle > 0) {
                    return "sd" + itos(turnSpeed) + ":" + itos(turnSpeed) + ":" + itos(turnSpeed) + ":0";
                }
            }
            break;
        case DRIBBLE_STATE:
            if (mBallCaptured) {
                mState = SHOOT_STATE;
            } else {
                return "sd-7:7:0:0\nd1";
            }
            break;
    }

    return getCommand();
}
