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

    /*
    // Useless for too long. do something about it
    if (mUselessTime > 10000) {
        mTimer = 0;
        mState = MOVE_TOWARDS_GOAL_STATE;
    }

    cout << mState << endl;
     */

    switch (mState) {
        case IDLE_STATE:
            if (mGameIsOn) {
                mState = CHOOSE_BALL_STATE;
            } else {
                mUselessTime = 0;
                return "sd0:0:0:0\nd100";
            }
            break;
        case CHOOSE_BALL_STATE:
            // A ball is already captured
            if (mBallCaptured) {
                mTimer = 0;
                mState = FIND_GOAL_STATE;
                return "d150";
            }

            mUselessTime += dt;

            Detector::Ball *closestBall;
            closestBall = getClosestBall();

            if (closestBall == NULL) {
                mState = FIND_BALLS_STATE;
            } else {
                //mTargetBall = *closestBall;
                mGoodAngleFrames = 0;
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

            mUselessTime += dt;

            // Back to finding balls if no balls are on the screen
            mTimer += dt;

            if (mTimer > 5000) {
                mTimer = 0;
                mState = MOVE_TOWARDS_GOAL_STATE;
            }

            if (mBalls.size()) {
                mTimer = 0;
                mGoodAngleFrames = 0;
                mState = GET_BALL_STATE;
            } else {
                return "sd25:25:25:0";
            }
            break;
        case SHOOT_STATE:
            mTimer += dt;
            //Detector::Ball* nextClosestBall = getClosestBall();

            if (!mKicked && mBallCaptured) {
                Detector::Ball* closestBall = getClosestBall();
                mTargetBallAvailable = closestBall != NULL;

                if (mTargetBallAvailable) {
                    mTargetBall = *closestBall;
                }
                //mNextClosestBallIsLeft = nextClosestBall != NULL && nextClosestBall->center.x < IMAGE_HALF_WIDTH;

                for (int i = 0; i < mBalls.size(); ++i) {
                    if (abs(mBalls[i].distance.x) < 3) {
                        mTimer = 0;
                        mState = MOVE_SIDEWAYS_STATE;
                        return "sd0";
                    }
                }

                mKicked = true;
                //mState = CHOOSE_BALL_STATE;
                return "k1250\nsd0\nd100";
            }

            if (mTimer < 600) {
                /*
                if (mTimer < 500) {
                    Detector::Ball* closestBall = getClosestBall();
                    mTargetBallAvailable = closestBall != NULL;

                    if (mTargetBallAvailable) {
                        mTargetBall = *closestBall;
                    }
                }
                 */

                if (!mTargetBallAvailable) {
                    return "sd20:20:20:0";
                }

                // Distance to ball
                float distance = sqrt(pow(mTargetBall.distance.x, 2) + pow(mTargetBall.distance.y - 4, 2));
                float angle = atan(mTargetBall.distance.x / (mTargetBall.distance.y + ROBOT_RADIUS));

                // PID
                float forwardSpeed = mForwardPid.tick(dt, distance);

                if (mTargetBall.distance.y < 4) {
                    forwardSpeed = 0;
                } else if (forwardSpeed < 20) {
                    forwardSpeed = 20;
                } else if (forwardSpeed > 100) {
                    forwardSpeed = 100;
                }

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
                return command;

            } else {
                mTimer = 0;
                mTurnPid.reset();
                mForwardPid.reset();
                mState = CHOOSE_BALL_STATE;
            }
        case MOVE_SIDEWAYS_STATE:
            if (mBallCaptured) {
                mTimer += dt;

                if (mTimer < 300) {
                    int forwardSpeed = 50;
                    float angle = -3.1415926 / 2;
                    float v0 = forwardSpeed * cos(WHEEL_0 - angle);
                    float v1 = forwardSpeed * cos(WHEEL_1 - angle);
                    float v2 = forwardSpeed * cos(WHEEL_2 - angle);
                    return "sd" + itos(v0) + ":" + itos(v1) + ":" + itos(v2);
                } else {
                    mTimer = 0;
                    mGoodAngleFrames = 0;
                    mState = FIND_GOAL_STATE;
                }
            } else {
                mTimer = 0;
                mState = CHOOSE_BALL_STATE;
            }
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
                mUselessTime += dt;

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
                float distance = sqrt(pow(ball->distance.x, 2) + pow(ball->distance.y - 4, 2));
                float angle = atan(ball->distance.x / (ball->distance.y + ROBOT_RADIUS));

                // PID
                float forwardSpeed = mForwardPid.tick(dt, distance);

                if (ball->distance.y < 4) {
                    forwardSpeed = 0;
                } else if (forwardSpeed < 20) {
                    forwardSpeed = 20;
                } else if (forwardSpeed > 100) {
                    forwardSpeed = 100;
                }

                //cout << forwardSpeed << endl;

                /*
                if (ball->distance.y <= 20 && forwardSpeed > 10) {
                    forwardSpeed = 10;
                }
                 */

                /*
                if (ball->distance.y <= 5) {
                    forwardSpeed /= 2;
                }*/

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

                // Ball angle
                if (ball->distance.x < 1) {
                    mGoodAngleFrames++;
                } else {
                    mGoodAngleFrames = 0;
                }

                // Dribbler status
                if (ball->distance.y < 30) {
                    command += "\nd150";

                    if (v2 < 1 && ball->distance.y <= 10 && mGoodAngleFrames > 5 ) {
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
                //return "sd0:0:0:0\nd100";
                mTimer = 0;
                mState = FIND_GOAL_STATE;
            } else {
                mTimer += dt;

                // Didn't get the ball after two seconds, should back up a little and try again
                if (mTimer > 1500) {
                    mTimer = 0;
                    mState = BACK_UP_STATE;
                    return "d100";
                }

                return "sd-20:20:0:0\nd150";
            }
            break;
        case BACK_UP_STATE:
            mTimer += dt;

            if (mTimer < 300) {
                return "sd50:-50:0:0";
            } else {
                mTimer = 0;
                mState = CHOOSE_BALL_STATE;
            }
            break;
        case FIND_GOAL_STATE:
            mUselessTime = 0;

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

                if (abs(distance.x) < mOpponentGoalWidth*0.7/2) {
                    mGoodAngleFrames++;
                } else {
                    mGoodAngleFrames = 0;
                }

                // Start timer when third wheel is slow
                if (speed < 10 && mTimer > 0) {
                    mTimer += dt;
                }

                // 10 degrees in radians
                if (speed < 10 && (mGoodAngleFrames > 3 || mTimer > 2000)) {
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
                    return "sd-50:50:0:0";
                } else {
                    //mTurnPid.reset();
                    mTimer = 0;
                    mState = FIND_GOAL_STATE;
                }
            } else if (mOwnGoalCenter.x == 0 && mOwnGoalCenter.y == 0) {
                mTurnPid.reset();

                if (mOpponentGoalIsLeft) {
                    return "sd14:14:70:0";
                } else {
                    return "sd-14:-14:-70:0";
                }
            } else {
                mTimer = dt;
            }
            break;
        // UNFINISHED, CURRENTLY NOT IN USE
        case MOVE_TOWARDS_GOAL_STATE:
            // Stop
            if (mTimer > 1500) {
                mTimer = 0;
                mUselessTime = 0;
                mState = CHOOSE_BALL_STATE;
                return "sd0";
            }

            // Find further goal
            if (mTimer == 0) {
                mTimer += dt;

                if (mOwnGoalCenter.x != 0 && mOwnGoalCenter.y != 0) {
                    FloatPoint distance = mDetector->getDistance(mOwnGoalCenter);
                    mMoveTowardsOpponentGoal = distance.y < 200;
                } else if (mOpponentGoalCenter.x != 0 && mOpponentGoalCenter.y != 0) {
                    FloatPoint distance = mDetector->getDistance(mOpponentGoalCenter);
                    mMoveTowardsOpponentGoal = distance.y > 200;
                } else {
                    return "sd30:30:30:0";
                }
            }

            // Get target goal center
            Point center;
            bool isLeft;

            if (mMoveTowardsOpponentGoal) {
                center = mOpponentGoalCenter;
                isLeft = mOpponentGoalIsLeft;
            } else {
                center = mOwnGoalCenter;
                isLeft = !mOpponentGoalIsLeft;
            }

            // Try to find target goal
            if (center.x == 0 && center.y == 0) {
                /*
                if (isLeft) {
                    return "sd-30:-30:-30:0";
                } else {
                    return "sd30:30:30:0";
                }
                 */
                return "sd30:30:30:0";
            }

            // Increase timer when goal is seen
            mTimer += dt;

            // Move towards target goal
            FloatPoint distance = mDetector->getDistance(center);

            //if (abs(distance.x) < 30) {
                return "sd-75:75:0:0";
            //}

            // Turn towards goal
            if (center.x < IMAGE_HALF_WIDTH) {
                return "sd-15:-15:-15:0";
            } else {
                return "sd15:15:15:0";
            }

            break;
    }

    return getCommand(dt);
}
