#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include <json/json.h>
#include <iostream>
#include <functional>
#include <thread>
#include <set>
#include "GPIO.h"
#include "ArduinoSensor.h"
#include <boost/asio.hpp>

typedef websocketpp::server<websocketpp::config::asio> server;
server websocket_server;
boost::asio::io_context io;
std::set<websocketpp::connection_hdl, std::owner_less<websocketpp::connection_hdl>> connections;
std::mutex connection_lock;
std::unique_ptr<ArduinoSensor> arduino_sensor;
std::unique_ptr<std::thread> arduino_thread;

void on_open(server* s, websocketpp::connection_hdl hdl)
{
    std::lock_guard<std::mutex> guard(connection_lock);
    connections.insert(hdl);
}

void on_close(server* s, websocketpp::connection_hdl hdl)
{
    std::lock_guard<std::mutex> guard(connection_lock);
    connections.erase(hdl);
    // Add any necessary cleanup or notification logic for closed connections
}

void on_message(server* s, websocketpp::connection_hdl hdl, server::message_ptr msg)
{
    std::cout << "Received message: " << msg->get_payload() << std::endl;

    // Parse the incoming JSON message
    Json::Value json;
    Json::Reader reader;
    if (reader.parse(msg->get_payload(), json)) {
        // Extract the GPIO pin number and state from the message
        int gpioPin = json["pin"].asInt();
        std::string gpioState = json["state"].asString();

        // Create a GPIO object for the specified pin
        GPIO gpio(gpioPin);

        // Set the GPIO state based on the received command
        if (gpioState == "HIGH") {
            gpio.setHigh();
            std::cout << "GPIO " << gpioPin << " set to HIGH" << std::endl;
        } else if (gpioState == "LOW") {
            gpio.setLow();
            std::cout << "GPIO " << gpioPin << " set to LOW" << std::endl;
        } else {
            std::cout << "Invalid GPIO state: " << gpioState << std::endl;
        }

        // Create a JSON response message
        Json::Value response;
        response["status"] = "success";
        response["message"] = "GPIO command executed";

        // Send the response back to the client
        s->send(hdl, Json::writeString(Json::StreamWriterBuilder(), response), websocketpp::frame::opcode::text);
    } else {
        std::cout << "Failed to parse JSON message" << std::endl;

        // Create an error response message
        Json::Value response;
        response["status"] = "error";
        response["message"] = "Invalid JSON message";

        // Send the error response back to the client
        s->send(hdl, Json::writeString(Json::StreamWriterBuilder(), response), websocketpp::frame::opcode::text);
    }
}

void run_server()
{
    websocket_server.set_open_handler(std::bind(&on_open, &websocket_server, std::placeholders::_1));
    websocket_server.set_close_handler(std::bind(&on_close, &websocket_server, std::placeholders::_1));
    websocket_server.set_message_handler(std::bind(&on_message, &websocket_server, std::placeholders::_1, std::placeholders::_2));

    try {
        websocket_server.init_asio();
        websocket_server.listen(9002);
        websocket_server.start_accept();
        websocket_server.run();
    } catch (const websocketpp::exception& e) {
        std::cerr << "WebSocket exception: " << e.what() << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Standard exception: " << e.what() << std::endl;
    } catch (...) {
        std::cerr << "Unknown exception occurred." << std::endl;
    }
}

void handle_arduino_data()
{
    Json::StreamWriterBuilder builder;
    builder["commentStyle"] = "None";
    builder["indentation"] = ""; // Compact JSON

    while (true) {
        Json::Value data = arduino_sensor->getLatestData();
        std::string message = Json::writeString(builder, data);

        {
            std::lock_guard<std::mutex> guard(connection_lock);
            for (auto hdl : connections) {
                websocket_server.send(hdl, message, websocketpp::frame::opcode::text);
            }
        }

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

void signal_handler(int signal)
{
    std::cout << "Received signal " << signal << ". Shutting down gracefully..." << std::endl;
    io.stop();
    websocket_server.stop();
    // kill the app
    exit(0);
}

int main()
{
    try {
        arduino_sensor = std::make_unique<ArduinoSensor>("/dev/serial/by-id/usb-Arduino__www.arduino.cc__0043_950373235353516030B1-if00", 9600, io);
        arduino_sensor->start();
        arduino_thread = std::make_unique<std::thread>(handle_arduino_data);

        // Set up signal handler for graceful shutdown
        std::signal(SIGINT, signal_handler);

        run_server();

        arduino_sensor->stop();
        arduino_thread->join();
    } catch (const std::exception& e) {
        std::cerr << "Exception in main: " << e.what() << std::endl;
    }

    return 0;
}