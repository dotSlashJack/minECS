// File: src/GPIO.cpp
#include "GPIO.h"
#include <wiringPi.h>  // Make sure to include the WiringPi library
#include <iostream>
#include <mutex>

std::mutex setup_mutex;
bool initialized = false;

GPIO::GPIO(int pin) : pinNumber(pin) {
    std::lock_guard<std::mutex> lock(setup_mutex);
    if (!initialized) {
        //wiringPiSetup();  // Setup the library only once
        wiringPiSetupGpio();
        initialized = true;
    }
    pinMode(pinNumber, OUTPUT);
}

GPIO::~GPIO() {
    // TODO add cleanup code here
}

void GPIO::setHigh() {
    std::cout << "Setting pin " << pinNumber << " to HIGH" << std::endl;
    digitalWrite(pinNumber, HIGH);
}

void GPIO::setLow() {
    std::cout << "Setting pin " << pinNumber << " to LOW" << std::endl;
    digitalWrite(pinNumber, LOW);
}
