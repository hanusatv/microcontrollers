/* mbed Microcontroller Library
 * Copyright (c) 2019 ARM Limited
 * SPDX-License-Identifier: Apache-2.0
 */

#include "mbed.h"
#include "DFRobot_RGBLCD.h"
#include <chrono>
#include <cstdint>
#include <cstdio>

using namespace std::chrono;


#define WAIT_TIME_MS 100
#define START_TIME 60s
#define ALARM_PERIOD 1000


BufferedSerial pc(USBTX, USBRX, 115200);
DFRobot_RGBLCD lcd(16, 2, D14, D15);
Timeout timeout;

DigitalOut led1(LED1);
InterruptIn btn_startpause(D0, PullDown);
InterruptIn btn_reset(D1, PullDown);
InterruptIn btn_add5(D2, PullDown);
InterruptIn btn_sub5(D4, PullDown);
PwmOut alarm (D3);

class MyClass {
public:
    microseconds startTime = START_TIME; 
    microseconds remainingTime = START_TIME;
    int state = 0;

    void printf(int option) {
        lcd.home();
        if (option == 0) {
            alarm.write(0.0);
            startTime < 10s ? lcd.printf("Time: 0%llu        ", startTime / 1000000): lcd.printf("Time: %llu        ", startTime / 1000000);
        }
        if (option == 1) {
            alarm.write(0.0);
            remainingTime < 10s ? lcd.printf("Time: 0%llu        ", remainingTime / 1000000): lcd.printf("Time: %llu        ", remainingTime / 1000000);
        } 
        if (option == 2) {
            alarm.write(0.5);
            lcd.printf("Eggs finished!");
        } 
    }
};
MyClass myclass;

////
//Interrupts
////
void interruptStartPause() {
    myclass.state = 2;
}

void interruptReset() {
    myclass.state = 3;
}

void interruptAdd5() {
    myclass.state = 5;
}

void interruptSub5() {
    myclass.state = 6;
}

////
//Functions
////
void endTimeout() {
    timeout.detach();
    myclass.startTime = myclass.remainingTime = START_TIME;
    myclass.state = 4;
}


void setup() {
    lcd.init();
    btn_startpause.fall(&interruptStartPause);
    btn_reset.fall(&interruptReset);
    btn_add5.fall(&interruptAdd5);
    btn_sub5.fall(&interruptSub5);
    alarm.period_us(ALARM_PERIOD);
}

int main() {
    setup();
    while (true)
    {
        switch (myclass.state) {
            case 0: { //Static print
                myclass.printf(0);
                break;
            }

            case 1: { //Running print
                myclass.remainingTime = timeout.remaining_time();
                myclass.printf(1);
                break;
            }

            case 2: { //StartPause interrupt
                if (myclass.startTime == myclass.remainingTime) {
                    timeout.attach(&endTimeout, myclass.startTime);
                    myclass.state = 1;
                } 
                else {
                    timeout.detach();
                    myclass.startTime = myclass.remainingTime;
                    myclass.state = 0;
                }
                break;
            }

            case 3: {//Reset
                timeout.detach();
                myclass.startTime = myclass.remainingTime = START_TIME;
                myclass.state = 0;
                break;
            }

            case 4: {//Finished
                myclass.printf(2);
                break;
            }

            case 5: {//Add 5
                auto dummy = myclass.startTime == myclass.remainingTime;
                if (dummy) {
                    myclass.startTime = myclass.remainingTime += 5s;
                    myclass.state = 0;
                }
                else {
                    myclass.startTime = timeout.remaining_time() + 5s;
                    timeout.detach();
                    timeout.attach(&endTimeout, myclass.startTime);
                    myclass.state = 1;
                }
                break;
            }

            case 6: {//Sub 5
                auto dummy = myclass.startTime == myclass.remainingTime;
                if (myclass.remainingTime <= 5s) {;
                    endTimeout();
                }
                else if (dummy) {
                    myclass.startTime = myclass.remainingTime -= 5s;
                    myclass.state = 0;
                }
                else {
                    myclass.startTime = timeout.remaining_time() - 5s;
                    timeout.detach();
                    timeout.attach(&endTimeout, myclass.startTime);
                    myclass.state = 1;
                }
                break;
            }
            default: {
                printf("Default\n");
                break;
            }
        }

        led1 = !led1;
        thread_sleep_for(WAIT_TIME_MS);
    }
}
