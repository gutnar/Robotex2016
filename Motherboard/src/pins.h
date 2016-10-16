#ifndef PINS_H_
#define PINS_H_
#include "mbed.h"

// MOTOR0 (J5)
#define M0_PWM P2_3
#define M0_DIR1 P0_21
#define M0_DIR2 P0_20
#define M0_ENCA P0_19
#define M0_ENCB P0_18
#define M0_FAULT P0_22

// MOTOR1 (J7)
#define M1_PWM P2_2
#define M1_DIR1 P0_15
#define M1_DIR2 P0_16
#define M1_ENCA P2_7
#define M1_ENCB P2_6
#define M1_FAULT P0_17

// MOTOR2 (J8)
#define M2_PWM P2_1
#define M2_DIR1 P0_24
#define M2_DIR2 P0_25
#define M2_ENCA P0_26
#define M2_ENCB P0_9
#define M2_FAULT P0_23

// MOTOR3 (J10)
#define M3_PWM P2_0
#define M3_DIR1 P0_7
#define M3_DIR2 P0_6
#define M3_ENCA P0_5
#define M3_ENCB P0_4
#define M3_FAULT P0_8

// Coilgun
#define C_CHARGE P0_10
#define C_KICK P0_11
#define C_DONE P1_29
#define PWM0 P2_5

// GPIO0 (J3)
#define ADC0 P1_30
#define ADC1 P1_31
#define PWM1 P2_4

// LED1
#define LED1R P4_28
#define LED1G P4_29
#define LED1B P2_8

// LED2
#define LED2R P0_28
#define LED2G P1_18
#define LED2B P0_27

// COM (J2)
#define COMTX P0_0
#define COMRX P0_1

#endif
