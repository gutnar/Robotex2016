//
// Created by jk on 28.11.16.
//

#include "PID.h"

PID::PID() {
}

PID::PID(float p, float i, float d) {
    mP = p;
    mI = i;
    mD = d;
}

float PID::tick(int dt, float error) {
    mIntegral += error * dt;
    float derivative = (error - mPreviousError) / dt;
    mPreviousError = error;

    return mP * error + mI * error + mD * derivative;
}

void PID::reset() {
    mIntegral = 0;
    mPreviousError = 0;
}