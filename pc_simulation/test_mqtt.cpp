
#include "test_mqtt.h"

namespace test_mqtt {

    WiFiClient wifiClient;
    PubSubClient mqttClient(wifiClient);

    const char* mqtt_server   = "127.0.0.1"; // public broker for testing
    const int   mqtt_port     = 1883;
    const char* mqtt_clientID = "DalhalSimWinClient";
    const char* mqtt_topic    = "dalhal/sim/test";
    // -----------------------------------------------------------------------------
    // Callback when a message is received
    // -----------------------------------------------------------------------------
    void mqttCallback(char* topic, byte* payload, unsigned int length) {
        std::cout << "\n[MQTT] Message received on topic " << topic << ": ";
        for (unsigned int i = 0; i < length; i++)
            std::cout << (char)payload[i];
        std::cout << std::endl;
    }

    // -----------------------------------------------------------------------------
    // Setup
    // -----------------------------------------------------------------------------
    void setup() {
        std::cout << "Connecting to MQTT broker: " << mqtt_server << "..." << std::endl;

        mqttClient.setServer(mqtt_server, mqtt_port);
        mqttClient.setCallback(mqttCallback);

        if (mqttClient.connect(mqtt_clientID)) {
            std::cout << "Connected to broker." << std::endl;
            mqttClient.subscribe(mqtt_topic);
            mqttClient.publish(mqtt_topic, "Hello from Windows Dalhal simulator!");
        } else {
            std::cerr << "Failed to connect to broker." << std::endl;
        }
    }

    // -----------------------------------------------------------------------------
    // Loop
    // -----------------------------------------------------------------------------
    void loop() {
        if (!mqttClient.connected()) {
            std::cerr << "[MQTT] Lost connection. Trying to reconnect..." << std::endl;
            mqttClient.connect(mqtt_clientID);
            mqttClient.subscribe(mqtt_topic);
        }
        mqttClient.loop();
        //std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}