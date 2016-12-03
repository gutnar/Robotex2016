//
// Created by jk on 13.10.16.
//

#include "AI.h"

AI::AI() {
}

AI::AI(Detector &detector) {
    mDetector = &detector;
}

void AI::notify(bool gameIsOn, vector<Detector::Ball> &balls, bool ballCaptured, Point opponentGoalCenter,
                Point ownGoalCenter, int opponentGoalWidth) {
    mGameIsOn = gameIsOn;
    mBalls = balls;
    mBallCaptured = ballCaptured;

    mOwnGoalCenter = ownGoalCenter;
    mOpponentGoalCenter = opponentGoalCenter;

    // Can see opponent goal
    if (opponentGoalCenter.x != 0 && opponentGoalCenter.y != 0) {
        mOpponentGoalIsLeft = opponentGoalCenter.x < IMAGE_HALF_WIDTH;
        mOpponentGoalWasLeft = mOpponentGoalIsLeft;
    }

    // Can see own goal
    else if (ownGoalCenter.x != 0 && ownGoalCenter.y != 0) {
        if (mOpponentGoalWasLeft == ownGoalCenter.x < IMAGE_HALF_WIDTH) {
            mOpponentGoalIsLeft = !mOpponentGoalWasLeft;
        } else {
            mOpponentGoalIsLeft = mOpponentGoalWasLeft;
        }
    }

    mOpponentGoalWidth = opponentGoalWidth;
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
                mTimer = 0;
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
                mTimer = 0;
                mState = FIND_GOAL_STATE;
                return "d150";
            }

            // Back to finding balls if no balls are on the screen
            mTimer += dt;

            if (mTimer > 5000) {
                mTimer = 0;
                mState = MOVE_TOWARDS_GOAL_STATE;
            }

            if (mBalls.size()) {
                mTimer = 0;
                mState = GET_BALL_STATE;
            } else {
                return "sd25:25:25:0";
            }
            break;
        case SHOOT_STATE:
            mTimer += dt;

            if (!mKicked && mBallCaptured) {
                mKicked = true;
                //mState = CHOOSE_BALL_STATE;
                return "k1250\nsd0\nd0";
            }

            if (mTimer > 1000) {
                mTimer = 0;
                mState = CHOOSE_BALL_STATE;
            }

            return "sd0";
            break;
        case GET_BALL_STATE:
            // A ball is already captured
            if (mBallCaptured) {
                mForwardPid.reset();
                mTurnPid.reset();
                mTimer = 0;
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
                float angle = atan(ball->distance.x / (ball->distance.y + ROBOT_RADIUS));

                // PID
                float forwardSpeed = mForwardPid.tick(dt, distance);

                if (forwardSpeed < 20) {
                    forwardSpeed = 20;
                }

                if (forwardSpeed > 75) {
                    forwardSpeed = 75;
                }

                //cout << forwardSpeed << endl;

                /*
                if (ball->distance.y <= 20 && forwardSpeed > 10) {
                    forwardSpeed = 10;
                }
                 */

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

                    if (v2 < 1 && ball->distance.y <= 10 && ball->distance.x <= 2) {
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
                mTimer = 0;
                mState = FIND_GOAL_STATE;
            } else {
                mTimer += dt;

                if (mTimer > 2000) {
                    mTimer = 0;
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
                mGoodAngleFrames = 0;
            } else if (mOpponentGoalCenter.x == 0 && mOpponentGoalCenter.y == 0) {
                mTurnPid.tick(dt, 3.1419/2);
                mGoodAngleFrames = 0;
                mTimer = 0;

                if (mOpponentGoalIsLeft) {
                    return "sd-14:-14:-70:0";
                } else {
                    return "sd14:14:70:0";
                }
            }

            // Check if shooting angle is fine
            else if (mOpponentGoalWidth <= 25 && abs(mOpponentGoalCenter.x - IMAGE_HALF_WIDTH) < 200) {
                mTurnPid.reset();
                mTimer = 0;
                mGoodAngleFrames = 0;
                mState = FIND_BETTER_SHOOTING_ANGLE_STATE;
            } else {
                // Get distance to goal
                FloatPoint distance = mDetector->getDistance(mOpponentGoalCenter);

                // PID
                float angle = atan(distance.x / (distance.y + ROBOT_RADIUS));
                //float angle = abs(mOpponentGoalCenter.x - IMAGE_HALF_WIDTH);
                float speed = mTurnPid.tick(dt, angle) * ROBOT_RADIUS;

                if (speed > 0 && mOpponentGoalCenter.x < IMAGE_HALF_WIDTH) {
                    speed *= -1;
                } else if (speed < 0 && mOpponentGoalCenter.x > IMAGE_HALF_WIDTH) {
                    speed *= -1;
                }

                if (speed > 30) {
                    speed = 30;
                } else if (speed < -30) {
                    speed = -30;
                }

                // Check if shooting angle is good
                if (abs(angle) < 5.0 / 180.0 * 3.1419) {
                    mGoodAngleFrames++;
                } else {
                    mGoodAngleFrames = 0;
                }

                // Start timer when third wheel is slow
                if (speed < 10 && mTimer > 0) {
                    mTimer += dt;
                }

                // 10 degrees in radians
                if (speed < 10 && (mGoodAngleFrames > 5 || mTimer > 2000)) {
                //if (speed < 10 && abs(distance.x) < mOpponentGoalWidth/4) {
                    mTurnPid.reset();
                    mTimer = 0;
                    mDribblerStopped = false;
                    mKicked = false;
                    mState = SHOOT_STATE;
                    mGoodAngleFrames = 0;

                    return "sd0";
                } else {
                    return "sd" + itos(speed / 5) + ":" + itos(speed / 5) + ":" + itos(speed) + ":0";
                }
            }
        case FIND_BETTER_SHOOTING_ANGLE_STATE:
            if (!mBallCaptured) {
                //mTurnPid.reset();
                mState = CHOOSE_BALL_STATE;
            } else if (mTimer > 0) {
                mTimer += dt;

                if (mTimer < 1500) {
                    return "sd-30:30:0:0";
                } else {
                    //mTurnPid.reset();
                    mTimer = 0;
                    mState = FIND_GOAL_STATE;
                }
            } else if (mOwnGoalCenter.x == 0 && mOwnGoalCenter.y == 0) {
                mTurnPid.reset();

                if (mOpponentGoalIsLeft) {
                    return "sd6:6:30:0";
                } else {
                    return "sd-6:-6:-30:0";
                }
            } else {
                mTimer = dt;
            }

                /*
                // Get distance to goal
                FloatPoint distance = mDetector->getDistance(mOpponentGoalCenter);

                // PID
                float angle = atan(distance.x / (distance.y + ROBOT_RADIUS));
                //float angle = abs(mOpponentGoalCenter.x - IMAGE_HALF_WIDTH);
                float speed = mTurnPid.tick(dt, angle) * ROBOT_RADIUS;

                if (speed > 0 && mOpponentGoalCenter.x < IMAGE_HALF_WIDTH) {
                    speed *= -1;
                } else if (speed < 0 && mOpponentGoalCenter.x > IMAGE_HALF_WIDTH) {
                    speed *= -1;
                }

                if (speed > 30) {
                    speed = 30;
                } else if (speed < -30) {
                    speed = -30;
                }

                // 10 degrees in radians
                if (abs(angle) < 5.0 / 180.0 * 3.1419) {
                    mTurnPid.reset();
                    mTimer = 0;
                    mDribblerStopped = false;
                    mKicked = false;
                    mState = SHOOT_STATE;

                    return "sd0";
                } else {
                    return "sd" + itos(speed / 5) + ":" + itos(speed / 5) + ":" + itos(speed) + ":0";
                }
                */
            break;
        // UNFINISHED, CURRENTLY NOT IN USE
        case MOVE_TOWARDS_GOAL_STATE:
            if (mOpponentGoalCenter.x == 0 && mOpponentGoalCenter.y == 0) {
                mIntegral = 0;

                if (mOpponentGoalIsLeft) {
                    return "sd-10:-10:-10:0";
                } else {
                    return "sd10:10:10:0";
                }
            }

            // pid
            double angle = abs(mOpponentGoalCenter.x - IMAGE_HALF_WIDTH);//180/3.14159*atan(mOpponentGoalCenter.x/mOpponentGoalCenter.y);
            mIntegral += angle * dt;
            float derivative = (angle - mPreviousError) / dt;
            float output = (0.05 * angle + 0.20 * angle + 0.05 * derivative) / 5;
            mPreviousError = angle;

            /*
            if (angle < IMAGE_HALF_WIDTH) {
                output *= -1;
            }
             */

            if (mOpponentGoalCenter.x < IMAGE_HALF_WIDTH && output > 0) {
                output *= -1;
            } else if (mOpponentGoalCenter.x > IMAGE_HALF_WIDTH && output < 0) {
                output *= -1;
            }

            if (!mTimer && angle > 250) {
                return "sd" + itos(output) + ":" + itos(output) + ":" + itos(output) + ":0";
            }

            mTimer += dt;

            if (mTimer > 2000) {
                mTimer = 0;
                mState = FIND_BALLS_STATE;
            } else {
                return "sd-25:25:0:0";
            }
    }

    return getCommand(dt);
}
