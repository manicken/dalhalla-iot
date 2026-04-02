/*
  Dalhalla IoT — Windows Port
  WebSocket API for Windows using SimpleWebSocketServer
  
  Provides cross-platform WebSocket server for home automation commands.

  Copyright (C) 2026 Jannik Svensson

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or 
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the 
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License 
  along with this program. If not, see <https://www.gnu.org/licenses/>.
*/

#pragma once

#include <string>
#include <functional>
#include <map>
#include <memory>
#include <thread>
#include <mutex>
#include <queue>
#include <set>

namespace DALHAL {

    // Callback type for sending responses to clients
    using ResponseCallback = std::function<void(const std::string&)>;

    // Command structure for queueing
    struct PendingCommand {
        std::string command;
        ResponseCallback callback;
    };

    class WebSocketAPI {
    public:
        // Initialize WebSocket server on specified port
        static void setup(int port = 82);

        // Shutdown the server
        static void shutdown();

        // Send message to all connected clients
        static void SendMessage(const std::string& msg);
        static void SendMessage(const char* msg);
        static void SendMessage(const char* source, const char* msg);

        // Check if server is running
        static bool isRunning();

        // Get number of connected clients
        static int getClientCount();

    private:
        static std::unique_ptr<class WebSocketServer> server;
        static std::mutex serverMutex;

        // Event handlers
        static void onClientConnect(int clientId, const std::string& clientIP);
        static void onClientDisconnect(int clientId);
        static void onMessage(int clientId, const std::string& message);
        static void onError(const std::string& error);

        // Helper to broadcast to all clients
        static void broadcastMessage(const std::string& msg);
    };

}
