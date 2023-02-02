/* mbed Microcontroller Library
 * Copyright (c) 2019 ARM Limited
 * SPDX-License-Identifier: Apache-2.0
 */

#include "mbed.h"
#include "DFRobot_RGBLCD.h"
#include "Timer.h"
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <ratio>
#include <typeinfo>

#define WAIT_TIME_MS 100

using namespace std::chrono;

Timer timer;

BufferedSerial pc(USBTX, USBRX, 115200);
DigitalOut led1(LED1);
DFRobot_RGBLCD lcd(16, 2, D14, D15);
InterruptIn btn_reset(D0, PullDown);
InterruptIn btn_pause(D1, PullDown);

int main()
{
    lcd.init();
    timer.start();
    printf("This is the bare metal blinky example running on Mbed OS %d.%d.%d.\n", MBED_MAJOR_VERSION, MBED_MINOR_VERSION, MBED_PATCH_VERSION);
    uint64_t timePassed;
    while (true)
    {
        lcd.setCursor(0, 0);
        timePassed = duration_cast<milliseconds>(timer.elapsed_time()).count();
        lcd.printf("Time is %.2f", timePassed/1000.0);
        led1 = !led1;
        thread_sleep_for(WAIT_TIME_MS);
    }
}
