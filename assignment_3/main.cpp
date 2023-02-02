/* mbed Microcontroller Library
 * Copyright (c) 2019 ARM Limited
 * SPDX-License-Identifier: Apache-2.0
 */

#include "mbed.h"
#include "HTS221Sensor.h"
#include "DFRobot_RGBLCD.h"
#include <cstdio>
#include <string>

#define WAIT_TIME_MS 200

BufferedSerial pc(USBTX, USBRX, 115200);

DigitalOut led1(LED1);
DFRobot_RGBLCD lcd(16, 2, D14, D15);
DevI2C i2c(PB_11, PB_10);
HTS221Sensor sensor(&i2c);
InterruptIn button(D0, PullDown);

bool state = true;
float data;

void interrupted(){
    state = !state;
}

void setColor(float measurement){
    if (state) {
        if (measurement < 20) {
            lcd.setRGB(0, 0, 255);
        } else if (measurement >= 20 && measurement <= 24) {
            lcd.setRGB(255, 94, 5);
        } else {
            lcd.setRGB(255, 0, 0);
        }
    } else {
        float fade = 255*(100-measurement)/100;
        lcd.setRGB(fade, fade, 255);
    }
}

void print_measurement(){
    lcd.setCursor(0,0);
    if (state) {
        sensor.get_temperature(&data);      
        lcd.printf("Temp: %.1f         ", data);  
    }
    else {
        sensor.get_humidity(&data);
        lcd.printf("Humi: %.1f         ", data);
    }
    setColor(data);
}


int main()
{
    lcd.init();
    sensor.init(NULL);
    sensor.enable();
    sensor.reset();
    button.rise(&interrupted);
    
    while (true)
    {
        print_measurement();
        led1 = !led1;
        thread_sleep_for(WAIT_TIME_MS);
    }
}