#include "mbed.h"
#include "pins.h"
#include "motor.h"
#include "RGBLed.hpp"
#include "USBSerial.h"

USBSerial serial;

Serial pc(USBTX, USBRX);
Serial com(COMTX, COMRX);

RGBLed led1(LED1R, LED1G, LED1B);
RGBLed led2(LED2R, LED2G, LED2B);

DigitalIn infrared(ADC0);

#define NUMBER_OF_MOTORS 4

Motor motors[NUMBER_OF_MOTORS] = {
  Motor(&pc, M0_PWM, M0_DIR1, M0_DIR2, M0_FAULT, M0_ENCA, M0_ENCB),
  Motor(&pc, M1_PWM, M1_DIR1, M1_DIR2, M1_FAULT, M1_ENCA, M1_ENCB),
  Motor(&pc, M2_PWM, M2_DIR1, M2_DIR2, M2_FAULT, M2_ENCA, M2_ENCB),
  Motor(&pc, M3_PWM, M3_DIR1, M3_DIR2, M3_FAULT, M3_ENCA, M3_ENCB)
};

PwmOut m0(M0_PWM);
PwmOut m1(M1_PWM);
PwmOut m2(M2_PWM);
PwmOut m3(M3_PWM);
PwmOut pwm0(PWM0);
PwmOut pwm1(PWM1);

void serialInterrupt();
void parseCommad(char *command);

Ticker pidTicker;
int pidTickerCount = 0;
static const float PID_FREQ = 60;

char buf[32];
int serialCount = 0;
bool serialData = false;

void pidTick() {
  for (int i = 0; i < NUMBER_OF_MOTORS; i++) {
    motors[i].pidTick();
  }

  if (pidTickerCount++ % 25 == 0) {
    led1.setBlue(!led1.getBlue());
  }
}

int main() {
  pidTicker.attach(pidTick, 1/PID_FREQ);
  //serial.attach(&serialInterrupt);

  // dribbler motor test
  pwm1.period_us(400);
  pwm1.pulsewidth_us(100);

  // Ball detector status
  int infraredStatus = -1;

  while (1) {
    if (serial.readable()) {
      buf[serialCount] = serial.getc();
      //serial.putc(buf[serialCount]);

      if (buf[serialCount] == '\n') {
        parseCommad(buf);
        serialCount = 0;
        memset(buf, 0, 32);
      } else {
        serialCount++;
      }
    }

    /// INFRARED DETECTOR
    int newInfraredStatus = infrared.read();

    if (newInfraredStatus != infraredStatus) {
      infraredStatus = newInfraredStatus;
      serial.printf("i%d\n", newInfraredStatus);
      led2.setGreen(infraredStatus);
    }
  }
}

void parseCommad(char *command) {
  // command == "sd14:16:10:30"
  if (command[0] == 's' && command[1] == 'd') {
    char * sd;

    for (int i = 0; i < NUMBER_OF_MOTORS; ++i) {
      sd = strtok(i ? NULL : command + 2, ":");
      motors[i].setSpeed((int16_t) atoi(sd));
    }
  }

  else if (command[0] == 'd') {
    if (command[1] == '0') {
      pwm1.pulsewidth_us(100);
    } else {
      pwm1.pulsewidth_us(150);
    }
    //pwm1.pulsewidth_us((int) atoi(command+1));
    //serial.printf("sending %d\n", (int) atoi(command+1));
  }

  else if (command[0] == 's' && command[1] == 'g') {
    serial.printf("%d:%d:%d:%d\n", motors[0].getSpeed(), motors[1].getSpeed(), motors[2].getSpeed(), motors[3].getSpeed());
  }

  else if (command[0] == 'r') {
    led1.setRed(!led1.getRed());
  }

  else if (command[0] == 'g') {
    led1.setGreen(!led1.getGreen());
  }

  else if (command[0] == 'b') {
    led1.setBlue(!led1.getBlue());
  }

  else if (command[0] == 'i') {
    serial.printf("%d\n", infrared.read());
  }
}
