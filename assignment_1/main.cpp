/* mbed Microcontroller Library
 * Copyright (c) 2019 ARM Limited
 * SPDX-License-Identifier: Apache-2.0
 */

#include "mbed.h"
#include <cstring>


DigitalOut led1(A0);
BufferedSerial pc(USBTX, USBRX, 115200);

int input;

int main()
{
    printf("This is the bare metal blinky example running on Mbed OS %d.%d.%d.\n", MBED_MAJOR_VERSION, MBED_MINOR_VERSION, MBED_PATCH_VERSION);
    char buffer[8] = {0};
    while (true)
    {
        pc.read(buffer, 1);
        printf("Current value is %c \n",buffer[0]);
        strcmp(buffer, "1") == 0 ? led1 = true : led1 = false;
    }
}
