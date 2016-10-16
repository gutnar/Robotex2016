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

void setSpeedToMotors(int motor1speed, int motor2speed, int motor3speed) {
  motors[0].setSpeed(motor1speed);
  motors[1].setSpeed(motor2speed);
  motors[2].setSpeed(motor3speed);
}


void pidTick() {
  for (int i = 0; i < NUMBER_OF_MOTORS; i++) {
    motors[i].pidTick();
  }
}

void toggleLed() {
  led1.setRed(!led1.getRed());
  //serial.printf("tick\n");
}

int main() {
  pidTicker.attach(pidTick, 1/PID_FREQ);
  //pidTicker.attach(toggleLed, 0.5);

  //serial.attach(&serialInterrupt);

  while (1) {
    if (serial.readable()) {
      buf[serialCount] = serial.getc();

      if (buf[serialCount] == '\n') {
        serial.printf(buf);
        parseCommad(buf);
        serialCount = 0;
        memset(buf, 0, 32);
      } else {
        serialCount++;
      }
    }
  }
}

void parseCommad (char *command) {
  //serial.printf("%s\n", command);

  // command == "sd14:16:10:30"
  if (command[0] == 's' && command[1] == 'd') {
    char * sd;

    for (int i = 0; i < NUMBER_OF_MOTORS; ++i) {
      sd = strtok(i ? NULL : command + 2, ":");
      motors[i].setSpeed((int16_t) atoi(sd));
    }
  }

  if (command[0] == 's') {
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

  /*
  if (command[0] == 's' && command[1] == 'd') {
  int16_t speed = atoi(command + 2);
  motors[0].pid_on = 1;
  motors[0].setSpeed(speed);
}
if (command[0] == 's') {
for (int i = 0; i < NUMBER_OF_MOTORS; i++) {
serial.printf("s%d:%d\n", i, motors[i].getSpeed());
}
} else if (command[0] == 'w' && command[1] == 'l') {
int16_t speed = atoi(command + 2);
motors[0].pid_on = 0;
if (speed < 0) motors[0].backward(-1*speed/255.0);
else motors[0].forward(speed/255.0);
} else if (command[0] == 'p' && command[1] == 'p') {
uint8_t pGain = atoi(command + 2);
motors[0].pgain = pGain;
} else if (command[0] == 'p' && command[1] == 'i') {
uint8_t iGain = atoi(command + 2);
motors[0].igain = iGain;
} else if (command[0] == 'p' && command[1] == 'd') {
uint8_t dGain = atoi(command + 2);
motors[0].dgain = dGain;
} else if (command[0] == 'p') {
char gain[20];
motors[0].getPIDGain(gain);
pc.printf("%s\n", gain);
}
*/
}
