#include "ArduinoSensor.h"
#include <iostream>
#include <sstream>
#include <thread>
#include <chrono>

ArduinoSensor::ArduinoSensor(const std::string& port, unsigned int baud_rate, boost::asio::io_context& io)
    : serial_(io, port)
{
    serial_.set_option(boost::asio::serial_port_base::baud_rate(baud_rate));
    is_running_ = false;
}

void ArduinoSensor::start()
{
    is_running_ = true;
    readSerial();
}

void ArduinoSensor::stop()
{
    is_running_ = false;
    if (serial_.is_open()) {
        serial_.close();
    }
}

void ArduinoSensor::readSerial()
{
    if (!serial_.is_open()) {
        std::cerr << "Serial port not open." << std::endl;
        return;
    }

    boost::asio::async_read_until(serial_, boost::asio::dynamic_buffer(incoming_data_), '\n',
        [this](const boost::system::error_code& error, size_t bytes_transferred) {
            handleRead(error, bytes_transferred);
        });
}

void ArduinoSensor::handleRead(const boost::system::error_code& error, size_t bytes_transferred)
{
    if (error) {
        std::cerr << "Read failed: " << error.message() << std::endl;
        if (error == boost::asio::error::eof || error == boost::asio::error::connection_reset) {
            // Connection lost or data stream interrupted
            stop();
            std::cerr << "Connection to Arduino lost. Attempting to reconnect..." << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(1)); // Add a delay before reconnecting
            start(); // Attempt to reconnect
        }
        return;
    }

    std::string line = incoming_data_.substr(0, bytes_transferred);
    incoming_data_.erase(0, bytes_transferred);
    latest_data_ = parseSensorData(line);

    if (is_running_) {
        readSerial();
    }
}

SensorData ArduinoSensor::parseSensorData(const std::string& data)
{
    SensorData result;
    std::istringstream ss(data);
    char comma;
    ss >> result.tc1 >> comma >> result.tc2 >> comma >> result.tc3 >> comma
       >> result.loadcell1 >> comma >> result.loadcell2 >> comma >> result.loadcell3 >> comma >> result.loadcell4;
    return result;
}

Json::Value ArduinoSensor::getLatestData()
{
    Json::Value data;
    data["tc1"] = latest_data_.tc1;
    data["tc2"] = latest_data_.tc2;
    data["tc3"] = latest_data_.tc3;
    data["loadcell1"] = latest_data_.loadcell1;
    data["loadcell2"] = latest_data_.loadcell2;
    data["loadcell3"] = latest_data_.loadcell3;
    data["loadcell4"] = latest_data_.loadcell4;
    return data;
}