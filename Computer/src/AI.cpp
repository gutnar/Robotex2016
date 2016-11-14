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

    if (goalCenter.x != 0 && goalCenter.y != 0) {
        mGoalWasLeft = goalCenter.x < IMAGE_HALF_WIDTH;
    }
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

string AI::getCommand(int dt)
{
    if (!mGameIsOn)
    {
        mState = IDLE_STATE;
    }

    switch (mState)
    {
        case IDLE_STATE:
            if (mGameIsOn)
            {
                mState = CHOOSE_BALL_STATE;
            } else
            {
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
            if (mBalls.size())
            {
                mState = GET_BALL_STATE;
            } else
            {
                return "sd10:10:10:0";
            }
            break;
        case SHOOT_STATE:
            if (!mDribblerStopped) {
                mDribblerStopped = true;
                return "d0";
            }

            if (!mKicked) {
                mKicked = true;
                return "k750";
            }

            mState = CHOOSE_BALL_STATE;
            return "sd0:0:0:0";
            /*
            // FIND GOAL
            if (mGoalCenter.x == 0 && mGoalCenter.y == 0)
            {
                return "sd10:10:10:0";
            } else
            {
                int difference = 5;
                if (mGoalCenter.x > IMAGE_HALF_WIDTH + difference)
                {
                    return "sd10:10:10:0";
                } else if (mGoalCenter.x < IMAGE_HALF_WIDTH - difference)
                {
                    return "sd-10:-10:-10:0";
                } else {
                    mState = CHOOSE_BALL_STATE;
                    return "d0\nk1000";
                }
            }
             */
            break;
        case GET_BALL_STATE:
            mTargetBall = getClosestBall();

            if (mTargetBall == NULL)
            {
                mState = FIND_BALLS_STATE;
            } else
            {
                // pid
                float error = mTargetBall->distance.x;
                mIntegral += error * dt;
                float derivative = (error - mPreviousError) / dt;
                float output = (0.05 * error + 0.20 * error + 0.05 * derivative) * 3;
                mPreviousError = error;

                float distance = sqrt(pow(mTargetBall->distance.x, 2) + pow(mTargetBall->distance.y, 2));

                if (distance < 15)
                {
                    if (abs(mTargetBall->distance.x) < 2)
                    {
                        mState = DRIBBLE_STATE;
                    } else
                    {
                        return "sd" + itos(2 * output) + ":" + itos(2 * output) + ":" + itos(2 * output) + ":0";
                    }
                } else
                {
                    return "sd" + itos(-20) + ":" + itos(20) + ":" + itos(output) + ":0";
                }

                /*
                double angle = atan((double) mTargetBall->distance.x / (mTargetBall->distance.y + 20)) * 180 / 3.14159;
                 */
            }
            break;
        case DRIBBLE_STATE:
            //if (mBallCaptured)
            mDribblerRuntime += dt;

            if (mDribblerRuntime > 4000)
            {
                mDribblerRuntime = 0;
                mState = FIND_GOAL_STATE;
            } else
            {
                return "sd-7:7:0:0\nd1";
            }
            break;
        case FIND_GOAL_STATE:
            if (mGoalCenter.x == 0 && mGoalCenter.y == 0)
            {
                if (mGoalWasLeft) {
                    return "sd10:10:10:0";
                } else {
                    return "sd-10:-10:-10:0";
                }
            } else
            {
                int difference = 5;

                if (mGoalCenter.x > IMAGE_HALF_WIDTH + difference)
                {
                    return "sd10:10:10:0";
                } else if (mGoalCenter.x < IMAGE_HALF_WIDTH - difference)
                {
                    return "sd-10:-10:-10:0";
                } else
                {
                    mDribblerRuntime += dt;

                    if (mDribblerRuntime > 1000)
                    {
                        mDribblerRuntime = 0;
                        mDribblerStopped = false;
                        mKicked = false;
                        mState = SHOOT_STATE;
                        return "sd0:0:0:0";
                    } else {
                        return "sd-5:5:0:0";
                    }
                }
            }
    }

    return getCommand(dt);
}
