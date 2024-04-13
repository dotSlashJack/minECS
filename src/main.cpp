#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include <json/json.h>
#include <iostream>
#include <functional>
#include <thread>
#include <set>
#include "GPIO.h"
#include "StateSequenceRunner.h"
#include "ArduinoSensor.h"
#include <boost/asio.hpp>
#include <wiringPi.h>

typedef websocketpp::server<websocketpp::config::asio> server;
server websocket_server;
boost::asio::io_context io;
std::set<websocketpp::connection_hdl, std::owner_less<websocketpp::connection_hdl>> connections;
std::mutex connection_lock;
std::unique_ptr<ArduinoSensor> arduino_sensor;
std::unique_ptr<std::thread> arduino_thread;
std::atomic<bool> server_running{true};

void on_open(server* s, websocketpp::connection_hdl hdl)
{
    std::lock_guard<std::mutex> guard(connection_lock);
    connections.insert(hdl);
    std::cout << "Client connected" << std::endl;
}

void on_close(server* s, websocketpp::connection_hdl hdl)
{
    std::lock_guard<std::mutex> guard(connection_lock);
    connections.erase(hdl);
    std::cout << "Client disconnected" << std::endl;
}

std::future<void> execute_sequence(const std::string& sequenceName, websocketpp::connection_hdl hdl, server* s)
{
    return std::async(std::launch::async, [sequenceName, hdl, s]() {
        StateSequenceRunner sequenceRunner;
        std::string result = sequenceRunner.sequenceRunner(sequenceName);

        // Create a JSON response message
        Json::Value response;
        response["status"] = "success";
        response["message"] = result;

        // Send the response back to the client
        s->send(hdl, Json::writeString(Json::StreamWriterBuilder(), response), websocketpp::frame::opcode::text);
    });
}

void on_message(server* s, websocketpp::connection_hdl hdl, server::message_ptr msg)
{
    std::cout << "Received message: " << msg->get_payload() << std::endl;

    // Parse the incoming JSON message
    Json::Value json;
    Json::Reader reader;
    if (reader.parse(msg->get_payload(), json)) {
        // Extract the command and sequence from the message
        std::string command = json["command"].asString();
        std::cout << "Command: " << command << std::endl;

        if (command == "START_SEQUENCE") {
            std::string sequenceName = json["sequence"].asString();
            // Use std::async to run the sequence asynchronously
            std::future<void> sequence_future = execute_sequence(sequenceName, hdl, s);
            // can store the future object if needed for later synchronization or error handling
            Json::Value response;
            response["status"] = "success";
            response["message"] = "Starting sequence...";

            // Send the response back to the client
            s->send(hdl, Json::writeString(Json::StreamWriterBuilder(), response), websocketpp::frame::opcode::text);

        } else if (command == "ABORT_SEQUENCE") {
            // Abort the currently running sequence and set the state to ONLINE_SAFE
            StateSequenceRunner sequenceRunner;
            sequenceRunner.abortSequence();

            // Create a JSON response message
            Json::Value response;
            response["status"] = "success";
            response["message"] = "Sequence aborted. State set to ONLINE_SAFE.";

            // Send the response back to the client
            s->send(hdl, Json::writeString(Json::StreamWriterBuilder(), response), websocketpp::frame::opcode::text);
        } else if (command == "SET_STATE") {
            std::string stateName = json["state"].asString();
            StateSequenceRunner sequenceRunner;
            sequenceRunner.executeState(stateName);

            // Create a JSON response message
            Json::Value response;
            response["status"] = "success";
            response["message"] = "State set to " + stateName;

            // Send the response back to the client
            s->send(hdl, Json::writeString(Json::StreamWriterBuilder(), response), websocketpp::frame::opcode::text);
        } else {
            std::cout << "Invalid command: " << command << std::endl;

            // Create an error response message
            Json::Value response;
            response["status"] = "error";
            response["message"] = "Invalid command";

            // Send the error response back to the client
            s->send(hdl, Json::writeString(Json::StreamWriterBuilder(), response), websocketpp::frame::opcode::text);
        }
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

        while (server_running) {
            websocket_server.run_one();
        }
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

    while (server_running) {
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
    server_running = false;
    io.stop();
    websocket_server.stop();
}

int main()
{

    try {
        arduino_sensor = std::make_unique<ArduinoSensor>("/dev/serial/by-id/usb-Arduino__www.arduino.cc__0043_950373235353516030B1-if00", 9600, io);
        arduino_sensor->start();
        arduino_thread = std::make_unique<std::thread>(handle_arduino_data);

        // shutdown cntrl c handler
        std::signal(SIGINT, signal_handler);

        run_server();

        arduino_sensor->stop();
        arduino_thread->join();
    } catch (const std::exception& e) {
        std::cerr << "Exception in main: " << e.what() << std::endl;
    }

    return 0;
}