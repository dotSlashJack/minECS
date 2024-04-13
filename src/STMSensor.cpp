#include "STMSensor.h"
#include <iostream>
#include <sstream>
#include <thread>
#include <chrono>

STMSensor::STMSensor(const std::string& port, unsigned int baud_rate, boost::asio::io_context& io)
    : serial_(io, port)
{
    serial_.set_option(boost::asio::serial_port_base::baud_rate(baud_rate));
    is_running_ = false;
}

void STMSensor::start()
{
    is_running_ = true;
    readSerial();
}

void STMSensor::stop()
{
    is_running_ = false;
    if (serial_.is_open()) {
        serial_.close();
    }
}

void STMSensor::readSerial()
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

void STMSensor::handleRead(const boost::system::error_code& error, size_t bytes_transferred)
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

SensorData STMSensor::parseSensorData(const std::string& data)
{
    SensorData result;
    std::istringstream ss(data);
    char comma;
    ss >> result.tc1 >> comma >> result.tc2 >> comma >> result.tc3 >> comma
       >> result.loadcell1 >> comma >> result.loadcell2 >> comma >> result.loadcell3 >> comma >> result.loadcell4;
    return result;
}

Json::Value STMSensor::getLatestData()
{
    Json::Value data;
    data["adc1"] = latest_data_.adc1;
    data["adc2"] = latest_data_.adc2;
    data["adc3"] = latest_data_.adc3;
    data["adc4"] = latest_data_.adc4;
    data["adc5"] = latest_data_.adc5;
    data["adc6"] = latest_data_.adc6;
    data["adc7"] = latest_data_.adc7;
    return data;
}