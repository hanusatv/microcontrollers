/* mbed Microcontroller Library
 * Copyright (c) 2019 ARM Limited
 * SPDX-License-Identifier: Apache-2.0
 */

#include "mbed.h"
#include <cstdio>
#include <string>
#include "HTS221Sensor.h"
#include "wifi.h"


// Blinking rate in milliseconds
#define BLINKING_RATE     500ms

constexpr uint32_t HTTP_REQUEST_BUFFER_SIZE = 300;
constexpr uint32_t JSON_CONTENT_BUFFER_SIZE = 100;
constexpr uint32_t HTTP_RESPONSE_BUFFER_SIZE = 400;

constexpr const char *ACCESS_TOKEN = "8wGMaCjjQpAuqvRQEf3B";
constexpr const char *HTTP_SERVER_ADDR = "192.168.205.146";
constexpr const uint16_t HTTP_SERVER_PORT = 9090;

DevI2C i2c(PB_11, PB_10);
HTS221Sensor sensor(&i2c);
NetworkInterface *network = NetworkInterface::get_default_instance();

float temp, humi;

void get_measurements() {
    sensor.get_temperature(&temp);
    sensor.get_humidity(&humi);
}

void setup() {
    sensor.init(NULL);
    sensor.enable();
}

int main()
{
    // Initialise the digital pin LED1 as an output
    DigitalOut led(LED1);
    if (!network) {
        printf("Failed to get default network interface\n");
        while (1);
    }

    nsapi_size_or_error_t result;

    do {
        printf("Connecting to the network...\n");
        result = network->connect();

        if (result != NSAPI_ERROR_OK) {
        printf("Failed to connect to network: %d\n", result);
        }
    } while (result != NSAPI_ERROR_OK);

    SocketAddress address;
    result = network->get_ip_address(&address);

    if (result != NSAPI_ERROR_OK) {
        printf("Failed to get local IP address: %d\n", result);
        while (1);
    }

    printf("Connected to WLAN and got IP address %s\n", address.get_ip_address());

    while (true) {
        get_measurements();
        printf("Humi: %.1f Temp: %.1f\n", humi, temp);
        led = !led;
        ThisThread::sleep_for(BLINKING_RATE);

        // Send HTTP POST request
        static char http_request[HTTP_REQUEST_BUFFER_SIZE];
        static char json_content[JSON_CONTENT_BUFFER_SIZE];

        snprintf(json_content, JSON_CONTENT_BUFFER_SIZE,
                "{\"temp\": %.1f, \"humi\": %.1f}", temp, humi);

        snprintf(http_request, HTTP_REQUEST_BUFFER_SIZE,
                "POST /api/v1/%s/telemetry HTTP/1.1\r\n"
                "Host: %s\r\n"
                "Connection: close\r\n"
                "Content-Type: application/json\r\n"
                "Content-Length: %u\r\n"
                "\r\n",
                ACCESS_TOKEN, HTTP_SERVER_ADDR, strlen(json_content));

        strcat(http_request, json_content);

        TCPSocket socket;

        result = socket.open(network);

        if (result != NSAPI_ERROR_OK) {
            printf("Failed to open TCPSocket: %d\n", result);
        continue;
        }

        result = network->gethostbyname(HTTP_SERVER_ADDR, &address);

        if (result != 0) {
            printf("Failed to set IP address of HTTP server: %d\n", result);
        continue;
        }

        address.set_port(HTTP_SERVER_PORT);
        result = socket.connect(address);

        if (result != 0) {
            printf("Failed to connect to HTTP server: %d\n", result);
        continue;
        }

        printf("\nSuccessfully connected to HTTP server @%s:%u\n\n",
            HTTP_SERVER_ADDR, HTTP_SERVER_PORT);

        result = send_request(&socket, http_request);

        if (result < 0) {
            printf("Sending HTTP POST failed with error code %d", result);
        continue;
        }

        // Receive HTTP POST response
        static char http_response[HTTP_RESPONSE_BUFFER_SIZE];
        result = read_response(&socket, http_response, HTTP_RESPONSE_BUFFER_SIZE);

        if (result < 0) {
            printf("Receive HTTP respons failed with error code %d", result);
        continue;
        }
    }
}
