//
// Created by jk on 28.11.16.
//

#ifndef ROBOTEX2016_PID_H
#define ROBOTEX2016_PID_H


class PID {
public:
    PID();
    PID(float p, float i, float d);
    float tick(int dt, float error);
    void reset();
private:
    float mPreviousError;
    float mIntegral;
    float mP = 0.05;
    float mI = 0.20;
    float mD = 0.05;
};


#endif //ROBOTEX2016_PID_H
