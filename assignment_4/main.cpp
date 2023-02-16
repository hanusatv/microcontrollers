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
#define TIMEOUT_MS 10000

using namespace std::chrono;

Timer timer;
BufferedSerial pc(USBTX, USBRX, 115200);
DigitalOut led1(LED1);
DFRobot_RGBLCD lcd(16, 2, D14, D15);
InterruptIn btn_reset(D0, PullDown);
InterruptIn btn_pause(D1, PullDown);

bool reset, pause, pauseTriggered = false;

void resetTrigger() {
    reset = true;
}

void pauseTrigger () {
    pauseTriggered = true;
}

void resetWatchdog() {
    Watchdog::get_instance().kick();
    reset = false;
}

void togglePause(Timer *timer) {
    pause = !pause;
    pauseTriggered = false;
    pause ? timer->stop() : timer->start();
}

void setup() {
    lcd.init();
    timer.start();
    btn_reset.fall(&resetTrigger);
    btn_pause.fall(&pauseTrigger);
    Watchdog::get_instance().start(TIMEOUT_MS);
}


int main()
{
    setup();
    uint64_t timePassed;
    while (true) {
        if (pause) {
            Watchdog::get_instance().kick();
        } else {
            lcd.setCursor(0, 0);
            timePassed = duration_cast<milliseconds>(timer.elapsed_time()).count();
            lcd.printf("Time is %.2f", timePassed/1000.0);
        }
        if (reset) resetWatchdog();
        if (pauseTriggered) togglePause(&timer);
        led1 = !led1;
        thread_sleep_for(WAIT_TIME_MS);
    }
}