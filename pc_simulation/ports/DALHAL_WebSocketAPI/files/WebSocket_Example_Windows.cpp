/*
  Dalhalla IoT — Windows Port Example
  
  Demonstrates basic usage of the Windows WebSocket API

  Copyright (C) 2026 Jannik Svensson
  GNU General Public License v3.0 or later
*/

#include "DALHAL_WebSocketAPI_Windows.h"
#include <iostream>
#include <thread>
#include <chrono>

using namespace DALHAL;

int main() {
    std::cout << "=== Dalhalla IoT WebSocket Server (Windows Port) ===" << std::endl;

    // Initialize WebSocket server on port 82
    WebSocketAPI::setup(82);

    if (!WebSocketAPI::isRunning()) {
        std::cerr << "Failed to start WebSocket server" << std::endl;
        return 1;
    }

    std::cout << "Server started. Waiting for connections..." << std::endl;
    std::cout << "Connect WebSocket client to ws://127.0.0.1:82/ws" << std::endl;
    std::cout << "Press Ctrl+C to exit\n" << std::endl;

    // Broadcast test message every 5 seconds
    int messageCount = 0;
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(5));

        int clientCount = WebSocketAPI::getClientCount();
        if (clientCount > 0) {
            std::string msg = "Server message #" + std::to_string(++messageCount) + 
                             " to " + std::to_string(clientCount) + " client(s)";
            WebSocketAPI::SendMessage(msg);
        }
    }

    WebSocketAPI::shutdown();
    return 0;
}
