// File: include/GPIO.h
#ifndef GPIO_H
#define GPIO_H

class GPIO {
public:
    GPIO(int pin);
    ~GPIO();
    void setHigh();
    void setLow();

private:
    int pinNumber;
};

#endif // GPIO_H
