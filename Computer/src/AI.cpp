//
// Created by jk on 13.10.16.
//

#include "AI.h"

AI::AI() {
}

AI::AI(Detector& detector) {
    mDetector = &detector;
}

void AI::notify(bool gameIsOn, vector<Detector::Ball> &balls, bool ballCaptured, Point goalCenter) {
    mGameIsOn = gameIsOn;
    mBalls = balls;
    mBallCaptured = ballCaptured;
    mGoalCenter = goalCenter;

    if (goalCenter.x != 0 && goalCenter.y != 0) {
        mGoalWasLeft = goalCenter.x < IMAGE_HALF_WIDTH;
    }
}

Detector::Ball *AI::getClosestBall() {
    Detector::Ball *closestBall = NULL;

    for (int i = 0; i < mBalls.size(); ++i) {
        if (closestBall == NULL || mBalls[i].center.y > closestBall->center.y) {
            closestBall = &mBalls[i];
        }
    }

    return closestBall;
}

string AI::getSpeedCommand(float v, float angle) {
    int v0 = v * cos(WHEEL_0 - angle);
    int v1 = v * cos(WHEEL_1 - angle);
    int v2 = v * cos(WHEEL_2 - angle);

    return "sd" + itos(v0) + ":" + itos(v1) + ":" + itos(v2);
}

string AI::getCommand(int dt) {
    if (!mGameIsOn) {
        mState = IDLE_STATE;
    }

    switch (mState) {
        case IDLE_STATE:
            if (mGameIsOn) {
                mState = CHOOSE_BALL_STATE;
            } else {
                return "sd0:0:0:0\nd0";
            }
            break;
        case CHOOSE_BALL_STATE:
            // A ball is already captured
            if (mBallCaptured) {
                mState = FIND_GOAL_STATE;
                return "d150";
            }

            Detector::Ball *closestBall;
            closestBall = getClosestBall();

            if (closestBall == NULL) {
                mState = FIND_BALLS_STATE;
            } else {
                mTargetBall = *closestBall;
                mState = GET_BALL_STATE;
            }
            break;
        case FIND_BALLS_STATE:
            // A ball is already captured
            if (mBallCaptured) {
                mState = FIND_GOAL_STATE;
                return "d150";
            }

            // Back to finding balls if no balls are on the screen
            mDribblerRuntime += dt;

            if (mDribblerRuntime > 5000) {
                mDribblerRuntime = 0;
                mState = MOVE_TOWARDS_GOAL_STATE;
            }

            if (mBalls.size()) {
                mDribblerRuntime = 0;
                mState = GET_BALL_STATE;
            } else {
                return "sd25:25:25:0";
            }
            break;
        case SHOOT_STATE:
            mDribblerRuntime += dt;

            if (!mKicked && mBallCaptured) {
                mKicked = true;
                //mState = CHOOSE_BALL_STATE;
                return "k1250\nsd0\nd0";
            }

            if (mDribblerRuntime > 1000) {
                mDribblerRuntime = 0;
                mState = CHOOSE_BALL_STATE;
            }

            return "sd0";
            break;
        case GET_BALL_STATE:
            // A ball is already captured
            if (mBallCaptured) {
                mForwardPid.reset();
                mTurnPid.reset();
                mState = FIND_GOAL_STATE;
                return "d150";
            }

            // Back to finding balls if no balls are on the screen
            else if (mBalls.size() == 0) {
                mForwardPid.reset();
                mTurnPid.reset();
                mState = FIND_BALLS_STATE;
            } else {
                // Find the ball that is most likely the target
                Detector::Ball *ball = getClosestBall();

                /*
                float minDifference = -1;

                for (int i = 0; i < mBalls.size(); ++i) {
                    float difference = pow(mBalls[i].distance.x - mTargetBall.distance.x, 2) +
                                       pow(mBalls[i].distance.y - mTargetBall.distance.y, 2);

                    if (minDifference == -1 || difference < minDifference) {
                        minDifference = difference;
                        ball = &mBalls[i];
                    }
                }

                mTargetBall = *ball;
                 */

                // Distance to ball
                float distance = sqrt(pow(ball->distance.x, 2) + pow(ball->distance.y, 2));
                float angle = atan(ball->distance.x/(ball->distance.y + ROBOT_RADIUS));

                // PID
                float forwardSpeed = mForwardPid.tick(dt, distance);

                if (forwardSpeed < 20) {
                    forwardSpeed = 20;
                }

                if (forwardSpeed > 75) {
                    forwardSpeed = 75;
                }

                cout << forwardSpeed << endl;

                // Forward
                float v0 = forwardSpeed * cos(WHEEL_0);// - angle);
                float v1 = forwardSpeed * cos(WHEEL_1);// - angle);
                float v2 = forwardSpeed * cos(WHEEL_2);// - angle);

                // Rotation
                float rotateSpeed = mTurnPid.tick(dt, angle);

                v0 += rotateSpeed * ROBOT_RADIUS;
                v1 += rotateSpeed * ROBOT_RADIUS;
                v2 += rotateSpeed * ROBOT_RADIUS;

                // Speed command
                string command = "sd" + itos(v0) + ":" + itos(v1) + ":" + itos(v2) + ":0";

                // Dribbler status
                if (ball->distance.y < 30) {
                    command += "\nd150";

                    if (v2 < 1 && ball->distance.y <= 10) {
                        mForwardPid.reset();
                        mTurnPid.reset();
                        mState = DRIBBLE_STATE;
                    }
                } else {
                    command += "\nd100";
                }

                return command;
            }
            break;
        case DRIBBLE_STATE:
            if (mBallCaptured) {
                //return "sd0:0:0:0\nd0";
                mState = FIND_GOAL_STATE;
            } else {
                mDribblerRuntime += dt;

                if (mDribblerRuntime > 2000) {
                    mDribblerRuntime = 0;
                    mState = CHOOSE_BALL_STATE;
                    return "d100";
                }

                return "sd-20:20:0:0\nd150";
            }
            break;
        case FIND_GOAL_STATE:
            if (!mBallCaptured) {
                mTurnPid.reset();
                mState = CHOOSE_BALL_STATE;
            } else if (mGoalCenter.x == 0 && mGoalCenter.y == 0) {
                mTurnPid.reset();

                if (mGoalWasLeft) {
                    return "sd-6:-6:-30:0";
                    //return "sd-10:-10:-10:0";
                } else {
                    //return "sd10:10:10:0";
                    return "sd6:6:30:0";
                }
            } else {
                // Get distance to goal
                FloatPoint distance = mDetector->getDistance(mGoalCenter);

                // PID
                float angle = atan(distance.x / (distance.y + ROBOT_RADIUS));
                //float angle = abs(mGoalCenter.x - IMAGE_HALF_WIDTH);
                float speed = mTurnPid.tick(dt, angle) * ROBOT_RADIUS;

                if (speed > 0 && mGoalCenter.x < IMAGE_HALF_WIDTH) {
                    speed *= -1;
                } else if (speed < 0 && mGoalCenter.x > IMAGE_HALF_WIDTH) {
                    speed *= -1;
                }

                if (speed > 30) {
                    speed = 30;
                } else if (speed < -30) {
                    speed = -30;
                }

                //cout << angle/3.1419*180.0 << " " << speed << endl;

                // 10 degrees in radians
                if (abs(angle) < 10/180.0*3.1419) {
                    mTurnPid.reset();
                    mDribblerRuntime = 0;
                    mDribblerStopped = false;
                    mKicked = false;
                    mState = SHOOT_STATE;

                    return "sd0";
                } else {
                    return "sd" + itos(speed / 5) + ":" + itos(speed / 5) + ":" + itos(speed) + ":0";
                }
            }
        // UNFINISHED, CURRENTLY NOT IN USE
        case MOVE_TOWARDS_GOAL_STATE:
            if (mGoalCenter.x == 0 && mGoalCenter.y == 0) {
                mIntegral = 0;

                if (mGoalWasLeft) {
                    return "sd-10:-10:-10:0";
                } else {
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

            if (mGoalCenter.x < IMAGE_HALF_WIDTH && output > 0) {
                output *= -1;
            } else if (mGoalCenter.x > IMAGE_HALF_WIDTH && output < 0) {
                output *= -1;
            }

            if (!mDribblerRuntime && angle > 250) {
                return "sd" + itos(output) + ":" + itos(output) + ":" + itos(output) + ":0";
            }

            mDribblerRuntime += dt;

            if (mDribblerRuntime > 2000) {
                mDribblerRuntime = 0;
                mState = FIND_BALLS_STATE;
            } else {
                return "sd-25:25:0:0";
            }
    }

    return getCommand(dt);
}
