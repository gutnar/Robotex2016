#ifndef MOTOR_H
#define  MOTOR_H
#include "mbed.h"
#include "PwmOut.h"

class Motor {
public:
    Motor() = default;
    Motor(Serial *pc, PinName pwm, PinName dir1, PinName dir2, PinName fault, PinName encA, PinName encB);
    Motor (const Motor& ) = default;

    uint8_t pid_on;
    int16_t encTicks;
    volatile int16_t speed;
    float pgain, igain, dgain;

    /*
     * Run this to update speed
     */
    void pid2(int16_t encTicks);
    int16_t getSpeed();
    void setSpeed (int16_t speed);
    void getPIDGain(char *gain);
    void forward(float pwm);
    void backward(float pwm);
    void pidTick();
private:
    static const uint PWM_PERIOD_US = 2500;

    Serial *_pc;
    PwmOut _pwm;
    DigitalOut _dir1;
    DigitalOut _dir2;
    DigitalIn _fault;
    InterruptIn _encA;
    InterruptIn _encB;

    volatile int16_t ticks;
    volatile uint8_t encNow;
    volatile uint8_t encLast;
    void encTick();

    union doublebyte {
        unsigned int value;
        unsigned char bytes[2];
    };

    uint8_t enc_dir;
    uint8_t enc_last;
    uint8_t enc_now;

    uint8_t dir;

    int16_t currentPWM;
    uint8_t motor_polarity;

    float pidSpeed;
    float pidError;
    float pidErrorPrev;
    float pidSetpoint;

    float i;

/*
    union doublebyte wcount;
    union doublebyte decoder_count;

    uint8_t dir;
*/

    uint8_t fail_counter;
    uint8_t send_speed;
    uint8_t failsafe;
    uint8_t leds_on;

    int16_t sp, sp_pid, der;
    int16_t intgrl, prop;
    //int16_t count, speed;
    int16_t csum;
    int16_t err, err_prev;

    int16_t pwm, pwm_min, pwmmin;
    uint8_t update_pid;

    uint16_t stall_counter;
    uint16_t stallCount;
    uint16_t prevStallCount;
    uint16_t stallWarningLimit;
    uint16_t stallErrorLimit;
    uint8_t stallLevel;
    uint8_t stallChanged;
    //int16_t currentPWM;

    uint8_t counter;


    void reset_pid();

};
#endif //MOTOR_H
