/* mbed Microcontroller Library
 * Copyright (c) 2019 ARM Limited
 * SPDX-License-Identifier: Apache-2.0
 */

#include "mbed.h"
#include <cstdio>

#define WAIT_TIME_MS 100
#define BLINK_TIME_MS 10

DigitalOut led0(LED1);

DigitalIn button(D0, PullDown);
AnalogIn potentiometer(A0);
PwmOut led1(D3);
PwmOut led2(D4);
PwmOut led3(D5);


int main()
{
    led1.period_ms(BLINK_TIME_MS);
    led2.period_ms(BLINK_TIME_MS);
    led3.period_ms(BLINK_TIME_MS);

    float potentiometer_value;
    while (true)
    {
        if (button.read()) {
            led1 = led2 = led3 = 1;
        }
        else {
            potentiometer_value = potentiometer.read();
            led1.write(potentiometer_value*3);
            led2.write((potentiometer_value - 0.33)*3);
            led3.write((potentiometer_value - 0.67)*3);
        }
        led0 = !led0;
        thread_sleep_for(WAIT_TIME_MS);
    }
}
