#ifndef CALIBRATION_H
#define CALIBRATION_H

#include <string>
#include <map>

// linearalibrations for the sensors
// in the form of {sensor_name, {slope, y-intercept}}
std::map<std::string, std::array<double, 2>> SensorCalibrations = {
    {"tc1", {1.0, 0.0}},
    {"tc2", {1.0, 0.0}},
    {"tc3", {1.0, 0.0}},
    {"loadcell1", {1.0, 0.0}},
    {"loadcell2", {1.0, 0.0}},
    {"loadcell3", {1.0, 0.0}},
    {"loadcell4", {1.0, 0.0}},
    {"adc1", {1.0, 0.0}},
    {"adc2", {1.0, 0.0}},
    {"adc3", {1.0, 0.0}},
    {"adc4", {1.0, 0.0}},
    {"adc5", {1.0, 0.0}},
    {"adc6", {1.0, 0.0}},
    {"adc7", {1.0, 0.0}},
};


#endif // CALIBRATION_H