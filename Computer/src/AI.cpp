//
// Created by jk on 13.10.16.
//

#include "AI.h"

AI::AI()
{

}

void AI::notify(vector<Detector::Ball> &balls, bool ballCaptured)
{
    mBalls = balls;
    mBallCaptured = ballCaptured;
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
    switch (mState)
    {
        case IDLE_STATE:
            mState = CHOOSE_BALL_STATE;
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
            //mState = CHOOSE_BALL_STATE;
            return "sd7:7:7:0";
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
                            return "sd-65:65:5:0";
                        } else if (mTargetBall->distance.x > 2) {
                            return "sd-65:65:-5:0";
                        } else {
                            return "sd-65:65:0:0";
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
