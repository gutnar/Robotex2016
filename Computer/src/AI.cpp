//
// Created by jk on 13.10.16.
//

#include "AI.h"

AI::AI()
{

}

void AI::notifyPositions(vector<Detector::Ball> &balls)
{
    mBalls = balls;
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
            mState = CHOOSE_BALL_STATE;
            break;
        case GET_BALL_STATE:
            //cout << mTargetBall->center.x << endl;
            mTargetBall = getClosestBall();

            if (mTargetBall == NULL) {
                mState = FIND_BALLS_STATE;
            } else {
                double angle = atan((double) mTargetBall->distance.x / (mTargetBall->distance.y + 20)) * 180 / 3.14159;

                cout << angle << endl;

                if (abs(angle) < 3) {
                    if (mTargetBall->distance.y < 5) {
                        return "sd0:0:0:0";
                    } else {
                        return "sd-10:10:0:0";
                        //return "sd0:0:0:0";
                    }
                }

                int turnSpeed = min((int) abs(angle), 10);

                if (angle < 0) {
                    return "sd-" + itos(turnSpeed) + ":-" + itos(turnSpeed) + ":-" + itos(turnSpeed) + ":0";
                }
  0) {
                    return "sd" + itos(turnSpeed) + ":" + itos(turnSpeed) + ":" + itos(turnSpeed) + ":0";
                }
            }
            break;
    }

    return getCommand();
}
