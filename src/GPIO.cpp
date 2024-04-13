// File: src/GPIO.cpp
#include "GPIO.h"
#include <wiringPi.h>  // Make sure to include the WiringPi library

GPIO::GPIO(int pin) : pinNumber(pin) {
    wiringPiSetup();       // Setup the library
    pinMode(pinNumber, OUTPUT);
}

GPIO::~GPIO() {
    // TODO add cleanup code here
}

void GPIO::setHigh() {
    digitalWrite(pinNumber, HIGH);
}

void GPIO::setLow() {
    digitalWrite(pinNumber, LOW);
}
