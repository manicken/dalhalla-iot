#ifndef WIFI_CONNECTION_MANAGER_H
#define WIFI_CONNECTION_MANAGER_H

#include <Arduino.h>
#if defined(ESP32)
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#endif

/**
 * @class WiFiConnectionManager
 * @brief Non-blocking WiFi manager with automatic reconnection and AP fallback
 *
 * Features:
 * - Auto-reconnect to last known WiFi with exponential backoff
 * - AP mode fallback after timeout
 * - Continues trying to reconnect even in AP mode with configurable interval
 * - Non-blocking - safe to call from main loop
 * - Minimal flash footprint (no WiFiManager dependency)
 * - Event callbacks for state changes
 */
class WiFiConnectionManager {
public:
    enum class State {
        IDLE,           // Not initialized
        STA_CONNECTING, // Attempting to connect to WiFi
        STA_CONNECTED,  // Connected to WiFi
        AP_RUNNING,     // Access Point active (fallback)
        RECONNECTING    // In AP mode, attempting WiFi reconnection
    };

    enum class Event {
        STA_CONNECT_SUCCESS,
        STA_CONNECT_FAILED,
        AP_STARTED,
        AP_TO_STA_SUCCESS,
        RECONNECT_ATTEMPT
    };

    // Callback function type
    using EventCallback = void (*)(Event event, const char* info);

    // ====================================================================
    // INITIALIZATION & CONTROL
    // ====================================================================

    /**
     * @brief Initialize WiFi manager and attempt connection
     * @param apSSID SSID for fallback AP mode (e.g., "Failsafe-Device")
     * @param apPassword Password for AP mode (min 8 chars)
     * @param staTimeout_ms Timeout for initial STA connection attempt (default 8000ms)
     * @param apTimeout_ms Time to wait in AP before starting reconnection attempts (default 60000ms)
     * @param reconnectInterval_ms Interval between reconnection attempts in AP mode (default 30000ms)
     */
    static void init(
        const char* apSSID = "ESP",
        const char* apPassword = "",
        unsigned long staTimeout_ms = 8000,
        unsigned long apTimeout_ms = 60000,
        unsigned long reconnectInterval_ms = 30000
    );

    /**
     * @brief Main task - call from your main loop regularly
     * Handles reconnection attempts, state transitions, etc.
     * Non-blocking and safe to call every loop iteration
     */
    static void task();

    /**
     * @brief Set callback for WiFi events
     */
    static void setEventCallback(EventCallback callback);

    // ====================================================================
    // STATE QUERIES
    // ====================================================================

    /**
     * @brief Get current state
     */
    static State getState();

    /**
     * @brief Check if connected to WiFi (STA mode)
     */
    static bool isConnected();

    /**
     * @brief Check if in AP fallback mode
     */
    static bool isAPActive();

    /**
     * @brief Get current WiFi SSID (empty if in AP mode)
     */
    static String getSSID();

    /**
     * @brief Get current local IP
     */
    static IPAddress getLocalIP();

    /**
     * @brief Get AP IP (valid only when in AP mode)
     */
    static IPAddress getAPIP();

    /**
     * @brief Get signal strength in dBm (valid only in STA mode)
     */
    static int8_t getSignalStrength();

    /**
     * @brief Get time since last successful connection (in seconds)
     */
    static unsigned long getTimeSinceLastConnection();

    /**
     * @brief Get next reconnection attempt time (for debugging)
     */
    static unsigned long getNextReconnectAttemptTime();

    /**
     * @brief Force manual reconnection attempt (useful for WS API commands)
     */
    static void forceReconnect();

    /**
     * @brief Manually force AP mode (e.g., for configuration)
     */
    static void forceAPMode();

    // ====================================================================
    // CONFIGURATION (call before init or after forceAPMode)
    // ====================================================================

    /**
     * @brief Change AP SSID and password
     */
    static void setAPCredentials(const char* ssid, const char* password);

    /**
     * @brief Change timeout values
     */
    static void setTimeouts(
        unsigned long staTimeout_ms,
        unsigned long apTimeout_ms,
        unsigned long reconnectInterval_ms
    );

private:
    static State currentState;
    static EventCallback eventCallback;

    // Timeouts and intervals (milliseconds)
    static unsigned long STA_TIMEOUT;
    static unsigned long AP_TIMEOUT;
    static unsigned long RECONNECT_INTERVAL;

    // Timing tracking
    static unsigned long stateChangeTime;      // When we entered current state
    static unsigned long lastReconnectAttempt;
    static unsigned long nextReconnectAttempt;
    static unsigned long lastSuccessfulConnection;

    // Exponential backoff
    static uint8_t reconnectAttemptCount;
    static const uint8_t MAX_BACKOFF_EXPONENT; // Limits exp backoff to ~32min

    // AP credentials
    static const char* apSSID;
    static const char* apPassword;

    // ====================================================================
    // INTERNAL STATE MACHINE
    // ====================================================================

    static void transitionToState(State newState);
    static void handleSTAConnecting();
    static void handleSTAConnected();
    static void handleAPRunning();
    static void handleReconnecting();

    static void emitEvent(Event event, const char* info = "");
    static unsigned long calculateBackoffDelay();

    // Helper to prevent WiFi.begin() spam
    static bool shouldAttemptReconnect();
};

#endif // WIFI_CONNECTION_MANAGER_H
