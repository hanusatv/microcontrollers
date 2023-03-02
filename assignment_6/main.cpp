/* mbed Microcontroller Library
 * Copyright (c) 2019 ARM Limited
 * SPDX-License-Identifier: Apache-2.0
 */

#include "mbed.h"
#include "wifi.h"
#include <cstdio>
#include <string>
#include "json.hpp"

#define WAIT_TIME_MS 500ms
#define DISCO_BLUE_BUTTON PC_13

static bool make_a_get_request = false;

static void blue_button_interrupt_cb() { make_a_get_request = true; }

int main() {
  DigitalOut led1(LED1); // GPIO PA_5
  InterruptIn button(DISCO_BLUE_BUTTON,
                     PullNone); // External pull-up

  button.fall(blue_button_interrupt_cb);

  // Get pointer to default network interface
  NetworkInterface *network = NetworkInterface::get_default_instance();

  if (!network) {
    printf("Failed to get default network interface\n");
    while (1)
      ;
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
    while (1)
      ;
  }

  printf("Connected to WLAN and got IP address %s\n", address.get_ip_address());

  while (true) {
    led1 = !led1; // Toggle "running" LED
    ThisThread::sleep_for(WAIT_TIME_MS);

    if (!make_a_get_request) {
      continue;
    }

    make_a_get_request = false;

    TCPSocket socket;

    // Configure timeout on socket receive
    // (returns NSAPI_ERROR_WOULD_BLOCK on timeout)
    socket.set_timeout(500);

    result = socket.open(network);

    if (result != NSAPI_ERROR_OK) {
      printf("Failed to open TCPSocket: %d\n", result);
      continue;
    }

    const char host[] = "www.mocky.io";
    result = network->gethostbyname(host, &address);

    if (result != NSAPI_ERROR_OK) {
      printf("Failed to get IP address of host %s: %d\n", host, result);
      continue;
    }

    printf("IP address of server %s is %s\n", host, address.get_ip_address());

    // Set server TCP port number
    address.set_port(80);

    // Connect to server at the given address
    result = socket.connect(address);

    // Check result
    if (result != NSAPI_ERROR_OK) {
        printf("Failed to connect to server at %s: %d\n", host, result);
        continue;
    }

    printf("Successfully connected to server %s\n", host);

    // Create HTTP request
    const char request[] = "GET /v2/5e37e64b3100004c00d37d03 HTTP/1.1\r\n"
                           "Host: mocky.io\r\n"
                           "Connection: close\r\n"
                           "\r\n";

    // Send request
    result = send_request(&socket, request);


    // Check result
    if (result < 0) {
        printf("Failed to send request: %d\n", result);
        continue;
    }

    // Buffer for HTTP responses
    static constexpr size_t HTTP_RESPONSE_BUF_SIZE = 2000;

    // Use static keyword to move response[] from stack to bss
    static char response[HTTP_RESPONSE_BUF_SIZE + 1]; // Plus 1 for '\0'
    // Read response
    result = read_response(&socket, response, HTTP_RESPONSE_BUF_SIZE);

    // Check result
    if (result < 0) {
        printf("Failed to read response: %d\n", result);
        continue;
    }

    // Take a look at the response, but before doing
    // so make sure we have a null-terminated string
    response[result] = '\0';
    printf("\nThe HTTP GET response:\n%s\n", response);
    
    int json_start_index = -1;
    char* json_start = strchr(response, '{');
    json_start_index = json_start - response;

    // Calculate the length of the JSON data by subtracting the start index
    // from the total length of the response buffer
    int json_length = strlen(response) - json_start_index;

    // Allocate a new buffer to hold the JSON data
    static char* json_data = new char[json_length + 1];

    // Copy the JSON data from the response buffer to the new buffer
    strncpy(json_data, response + json_start_index, json_length);

    // Add a null terminator to the end of the new buffer
    json_data[json_length] = '\0';

    // Parse the JSON data into a json object
    const static nlohmann::json json_object = nlohmann::json::parse(json_data);

    // Access data values from JSON object
    string firstName = json_object["first name"];
    string lastName = json_object["last name"];
    int age = json_object["age"];

    // Print first name, last name and age
    printf("First name: %s\n",firstName.c_str());
    printf("Last name: %s\n",lastName.c_str());
    printf("Age: %i\n",age);
  }
}