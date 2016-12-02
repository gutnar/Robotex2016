#include "motor.h"

Motor::Motor(Serial *pc, PinName pwm, PinName dir1, PinName dir2, PinName fault, PinName encA, PinName encB) :
    _pwm(pwm),
    _dir1(dir1),
    _dir2(dir2),
    _fault(fault),
    _encA(encA),
    _encB(encB),
    _pc(pc)
{
    _encA.mode(PullNone);
    _encB.mode(PullNone);

    ticks = 0;
    encNow = 0;
    encLast = 0;

    enc_last = 0;

    pid_on = 1;
    currentPWM = 0;

    encTicks = 0;
    motor_polarity = 0;

    pidSpeed = 0;
    pidError = 0;
    pidErrorPrev = 0;
    pidSetpoint = 0;

    i = 0;
    counter = 0;

    _pwm.period_us(PWM_PERIOD_US);

    _encA.rise(this, &Motor::encTick);
    _encA.fall(this, &Motor::encTick);
    _encB.rise(this, &Motor::encTick);
    _encB.fall(this, &Motor::encTick);

    //pidTicker.attach(this, &Motor::pidTick, 1/PID_FREQ);

    dir = 0;
    motor_polarity = 0;
    pgain = 0.02;
    igain = 0.005;
    dgain = 0;
    pwm_min = 25;
    intgrl = 0;
    //count = 0;
    speed = 0;
    err = 0;
}

void Motor::forward(float pwm) {
    if (dir) {
        _dir1 = 0;
        _dir2 = 1;
    } else {
        _dir1 = 1;
        _dir2 = 0;
    }

    _pwm = pwm;
    currentPWM = pwm;
}

void Motor::backward(float pwm) {
    if (dir) {
        _dir1 = 1;
        _dir2 = 0;
    } else {
        _dir1 = 0;
        _dir2 = 1;
    }

    _pwm = pwm;
    currentPWM = -pwm;
}

void Motor::pid2(int16_t encTicks) {
    speed = encTicks;

    if (!pid_on) return;
    pidError = pidSetpoint - speed;
    float p = pgain * pidError;
    i += igain * pidError;
    float d = dgain * (pidError - pidErrorPrev);
    pidSpeed = p + i + d;

    if (i > 1) {
      i = 1;
    } else if (i < -1) {
      i = -1;
    }

    pidErrorPrev = pidError;

    if (pidSpeed > 1) pidSpeed = 1;
    if (pidSpeed < -1) pidSpeed = -1;

    if (pidSpeed > 0) forward(pidSpeed);
    else backward(-pidSpeed);

    if (pidSetpoint == 0) {
        forward(0);
        pidError = 0;
        pidSpeed = 0;
    }
}

void Motor::reset_pid() {
    err = 0;
    err_prev = 0;
    i = 0;
    der = 0;
    sp = 0;
    sp_pid = 0;
    forward(0);
}

int16_t Motor::getSpeed() {
    return speed;
}

void Motor::setSpeed(int16_t speed) {
    sp_pid = speed;
    pidSetpoint = speed;
    if (sp_pid == 0) reset_pid();
    fail_counter = 0;
}

void Motor::getPIDGain(char *gain) {
    sprintf(gain, "PID:%f,%f,%f", pgain, igain, dgain);
}

void Motor::pidTick() {
    pid2(ticks);
    ticks = 0;
}

void Motor::encTick() {
    uint8_t enc_dir;
    encNow = _encA.read() | (_encB.read() << 1);
    enc_dir = (encLast & 1) ^ ((encNow & 2) >> 1);
    encLast = encNow;

    if (enc_dir & 1) ticks++;
    else ticks--;
}
