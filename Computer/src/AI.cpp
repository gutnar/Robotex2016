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

    if (goalCenter.x != 0 && goalCenter.y != 0)
    {
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
            mDribblerRuntime += dt;

            if (mDribblerRuntime > 5000)
            {
                mDribblerRuntime = 0;
                mState = MOVE_TOWARDS_GOAL_STATE;
            }

            if (mBalls.size())
            {
                mDribblerRuntime = 0;
                mState = GET_BALL_STATE;
            } else
            {
                return "sd25:25:25:0";
            }
            break;
        case SHOOT_STATE:
            /*
            if (!mDribblerStopped)
            {
                mDribblerStopped = true;
                return "sd0:0:0:0\nd0";
            }
             */

            mDribblerRuntime += dt;

            if (!mKicked)
            {
                mKicked = true;
                mState = CHOOSE_BALL_STATE;
                return "k1500\nd0\nsd0:0:0:0";

                /*
                mDribblerRuntime += dt;

                if (mDribblerRuntime > 0) {
                    mKicked = true;
                    mDribblerRuntime = 0;
                    return "k750";
                } else {
                    return "sd0:0:0:0";
                }
                 */
            }

            if (mDribblerRuntime > 1000)
            {
                mDribblerRuntime = 0;
                mState = CHOOSE_BALL_STATE;
            }

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
                /*
                float error = mTargetBall->distance.x;
                mIntegral += error * dt;
                float derivative = (error - mPreviousError) / dt;
                float output = (0.05 * error + 0.20 * error + 0.05 * derivative) * 3;
                mPreviousError = error;
                 */
                if (mGoalCenter.x > 0 && mGoalCenter.y > 0) {
                }


                int forwardSpeed = (int) mForwardPid.tick(dt, mTargetBall->distance.y) * 4;
                int turnSpeed = (int) mTurnPid.tick(dt, mTargetBall->distance.x) * 3;

                if (forwardSpeed > 100) {
                    forwardSpeed = 100;
                }

                float distance = sqrt(pow(mTargetBall->distance.x, 2) + pow(mTargetBall->distance.y, 2));

                //cout << mTargetBall->distance.x << endl;

                if (mTargetBall->distance.y < 15)
                {
                    if (turnSpeed == 0 || abs(mTargetBall->distance.x) < 0.5)
                    {
                        mState = DRIBBLE_STATE;
                    } else
                    {
                        return "sd" + itos(turnSpeed) + ":" + itos(turnSpeed) + ":" + itos(turnSpeed) + ":0";
                    }
                } else
                {
                    return "sd" + itos(-forwardSpeed) + ":" + itos(forwardSpeed) + ":" + itos(turnSpeed) + ":0";
                }

                /*
                double angle = atan((double) mTargetBall->distance.x / (mTargetBall->distance.y + 20)) * 180 / 3.14159;
                 */
            }
            break;
        case DRIBBLE_STATE:
            if (mBallCaptured)
            {
                mState = FIND_GOAL_STATE;
            } else
            {
                mDribblerRuntime += dt;

                if (mDribblerRuntime > 3000)
                {
                    mDribblerRuntime = 0;
                    mState = CHOOSE_BALL_STATE;
                    return "d0";
                }

                /*
                int turnSpeed = (int) mTurnPid.tick(dt, mTargetBall->distance.x) * 3;
                return "sd-10:10:" + itos(turnSpeed) + ":0\nd1";
                 */

                return "sd-10:10:0:0\nd1";
            }
            break;
        case FIND_GOAL_STATE:
            if (mGoalCenter.x == 0 && mGoalCenter.y == 0)
            {
                mTurnPid.reset();

                if (mGoalWasLeft)
                {
                    return "sd-10:-10:-10:0";
                } else
                {
                    return "sd10:10:10:0";
                }
            } else
            {
                /*
                int difference = 5;

                if (mGoalCenter.x > IMAGE_HALF_WIDTH + difference)
                {
                    return "sd10:10:10:0";
                } else if (mGoalCenter.x < IMAGE_HALF_WIDTH - difference)
                {
                    return "sd-10:-10:-10:0";
                } else
                {
                    mDribblerStopped = false;
                    mKicked = false;
                    mState = SHOOT_STATE;
                    return "sd0:0:0:0";
                }
                 */

                // pid
                double angle = abs(mGoalCenter.x - IMAGE_HALF_WIDTH);//180/3.14159*atan(mGoalCenter.x/mGoalCenter.y);
                mIntegral += angle * dt;
                float derivative = (angle - mPreviousError) / dt;
                float output = (0.05 * angle + 0.20 * angle + 0.05 * derivative) / 5;
                mPreviousError = angle;

                /*
                if (angle < IMAGE_HALF_WIDTH) {
                    output *= -1;
                }
                 */

                if (mGoalCenter.x < IMAGE_HALF_WIDTH && output > 0)
                {
                    output *= -1;
                } else if (mGoalCenter.x > IMAGE_HALF_WIDTH && output < 0)
                {
                    output *= -1;
                }

                if (angle > 25)
                {
                    return "sd" + itos(output) + ":" + itos(output) + ":" + itos(output) + ":0";
                } else
                {
                    mIntegral = 0;
                    mDribblerStopped = false;
                    mKicked = false;
                    mState = SHOOT_STATE;

                    return "sd0:0:0:0";
                }

            }
        case MOVE_TOWARDS_GOAL_STATE:
            if (mGoalCenter.x == 0 && mGoalCenter.y == 0)
            {
                mIntegral = 0;

                if (mGoalWasLeft)
                {
                    return "sd-10:-10:-10:0";
                } else
                {
                    return "sd10:10:10:0";
                }
            }

            // pid
            double angle = abs(mGoalCenter.x - IMAGE_HALF_WIDTH);//180/3.14159*atan(mGoalCenter.x/mGoalCenter.y);
            mIntegral += angle * dt;
            float derivative = (angle - mPreviousError) / dt;
            float output = (0.05 * angle + 0.20 * angle + 0.05 * derivative) / 5;
            mPreviousError = angle;

            /*
            if (angle < IMAGE_HALF_WIDTH) {
                output *= -1;
            }
             */

            if (mGoalCenter.x < IMAGE_HALF_WIDTH && output > 0)
            {
                output *= -1;
            } else if (mGoalCenter.x > IMAGE_HALF_WIDTH && output < 0)
            {
                output *= -1;
            }

            if (!mDribblerRuntime && angle > 250)
            {
                return "sd" + itos(output) + ":" + itos(output) + ":" + itos(output) + ":0";
            }

            mDribblerRuntime += dt;

            if (mDribblerRuntime > 2000) {
                mDribblerRuntime = 0;
                mState = FIND_BALLS_STATE;
            } else
            {
                return "sd-25:25:0:0";
            }
    }

    return getCommand(dt);
}
