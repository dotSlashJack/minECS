#ifndef ARDUINO_SENSOR_H
#define ARDUINO_SENSOR_H

#include <boost/asio.hpp>
#include <string>
#include <json/json.h>

struct SensorData {
    double tc1;
    double tc2;
    double tc3;
    double loadcell1;
    double loadcell2;
    double loadcell3;
    double loadcell4;
};

class ArduinoSensor {
public:
    ArduinoSensor(const std::string& port, unsigned int baud_rate, boost::asio::io_context& io);
    void start();
    void stop();
    Json::Value getLatestData();

private:
    void readSerial();
    void handleRead(const boost::system::error_code& error, size_t bytes_transferred);
    SensorData parseSensorData(const std::string& data);

    boost::asio::serial_port serial_;
    std::string incoming_data_;
    SensorData latest_data_;
    bool is_running_;
};

#endif