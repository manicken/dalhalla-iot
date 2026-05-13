#include "WiFiConnectionManager.h"

// ============================================================================
// STATIC MEMBER INITIALIZATION
// ============================================================================

WiFiConnectionManager::State WiFiConnectionManager::currentState = State::IDLE;
WiFiConnectionManager::EventCallback WiFiConnectionManager::eventCallback = nullptr;

unsigned long WiFiConnectionManager::STA_TIMEOUT = 8000;
unsigned long WiFiConnectionManager::AP_TIMEOUT = 60000;
unsigned long WiFiConnectionManager::RECONNECT_INTERVAL = 30000;

unsigned long WiFiConnectionManager::stateChangeTime = 0;
unsigned long WiFiConnectionManager::lastReconnectAttempt = 0;
unsigned long WiFiConnectionManager::nextReconnectAttempt = 0;
unsigned long WiFiConnectionManager::lastSuccessfulConnection = 0;

uint8_t WiFiConnectionManager::reconnectAttemptCount = 0;
const uint8_t WiFiConnectionManager::MAX_BACKOFF_EXPONENT = 10; // ~17 min max

const char* WiFiConnectionManager::apSSID = nullptr;
const char* WiFiConnectionManager::apPassword = nullptr;

// ============================================================================
// INITIALIZATION
// ============================================================================

void WiFiConnectionManager::init(
    const char* apSSID,
    const char* apPassword,
    unsigned long staTimeout_ms,
    unsigned long apTimeout_ms,
    unsigned long reconnectInterval_ms
) {
    if (currentState != State::IDLE) {
        Serial.println(F("[WiFiMgr] Already initialized!"));
        return;
    }

    // Store AP credentials
    WiFiConnectionManager::apSSID = apSSID;
    WiFiConnectionManager::apPassword = apPassword;

    // Store timeout values
    STA_TIMEOUT = staTimeout_ms;
    AP_TIMEOUT = apTimeout_ms;
    RECONNECT_INTERVAL = reconnectInterval_ms;

    Serial.print(F("[WiFiMgr] Initializing WiFi (STA timeout:")); Serial.print(STA_TIMEOUT); Serial.print(F("ms, AP timeout:")); Serial.print(AP_TIMEOUT); Serial.println(F("ms)")); 

    // Start connection attempt
    WiFi.mode(WIFI_STA);
    WiFi.begin();  // Uses stored credentials
    
    transitionToState(State::STA_CONNECTING);
    stateChangeTime = millis();
    reconnectAttemptCount = 0;
}

// ============================================================================
// MAIN TASK - Call from main loop
// ============================================================================

void WiFiConnectionManager::task() {
    if (currentState == State::IDLE) {
        return; // Not initialized
    }

    //unsigned long now = millis();

    switch (currentState) {
        case State::STA_CONNECTING:
            handleSTAConnecting();
            break;

        case State::STA_CONNECTED:
            handleSTAConnected();
            break;

        case State::AP_RUNNING:
            handleAPRunning();
            break;

        case State::RECONNECTING:
            handleReconnecting();
            break;

        default:
            break;
    }
}

// ============================================================================
// STATE MACHINE HANDLERS
// ============================================================================

void WiFiConnectionManager::handleSTAConnecting() {
    unsigned long now = millis();
    unsigned long elapsed = now - stateChangeTime;

    // Check if WiFi connected
    if (WiFi.status() == WL_CONNECTED) {
        transitionToState(State::STA_CONNECTED);
        lastSuccessfulConnection = now;
        reconnectAttemptCount = 0;
        
        char info[64];
        snprintf(info, sizeof(info), "%s (%s)", 
                 WiFi.SSID().c_str(), 
                 WiFi.localIP().toString().c_str());
        emitEvent(Event::STA_CONNECT_SUCCESS, info);
        Serial.print(F("[WiFiMgr] Connected to WiFi: ")); Serial.println(info);
        return;
    }

    // Timeout reached, switch to AP mode
    if (elapsed >= STA_TIMEOUT) {
        emitEvent(Event::STA_CONNECT_FAILED, "Connection timeout");
        Serial.print(F("[WiFiMgr] STA connection failed after")); Serial.print(STA_TIMEOUT); Serial.println(F("ms → Starting AP"));
        
        WiFi.mode(WIFI_AP);
        WiFi.softAP(apSSID, apPassword);
        
        transitionToState(State::AP_RUNNING);
        nextReconnectAttempt = millis() + AP_TIMEOUT; // Wait before first reconnect
        
        char info[64];
        snprintf(info, sizeof(info), "%s (%s)", apSSID, WiFi.softAPIP().toString().c_str());
        emitEvent(Event::AP_STARTED, info);
        Serial.print(F("[WiFiMgr] AP started: ")); Serial.println(info);
    }
}

void WiFiConnectionManager::handleSTAConnected() {
    unsigned long now = millis();

    // Check if still connected
    if (WiFi.status() != WL_CONNECTED) {
        Serial.print(F("[WiFiMgr] Lost WiFi connection! Attempting reconnect...\n"));
        WiFi.disconnect(false); // Keep config, don't turn off radio
        WiFi.begin();
        
        transitionToState(State::STA_CONNECTING);
        reconnectAttemptCount = 0;
        return;
    }

    // Periodically log signal strength (optional)
    static unsigned long lastSignalLog = 0;
    if (now - lastSignalLog > (5*60*1000)) { // Log every 5 minute
        lastSignalLog = now;
        int8_t rssi = WiFi.RSSI();
        Serial.print(F("[WiFiMgr] Signal strength:")); Serial.print(rssi); Serial.println(F("dBm"));
    }
}

void WiFiConnectionManager::handleAPRunning() {
    unsigned long now = millis();
    //unsigned long elapsed = now - stateChangeTime;

    // Try to reconnect at intervals
    if (shouldAttemptReconnect()) {
        Serial.print(F("[WiFiMgr] Attempting WiFi reconnect from AP mode (attempt #")); Serial.print(reconnectAttemptCount + 1); Serial.println(')');
        
        WiFi.mode(WIFI_STA);
        WiFi.begin();
        
        transitionToState(State::STA_CONNECTING);
        lastReconnectAttempt = now;
        emitEvent(Event::RECONNECT_ATTEMPT, "");
    }
}

void WiFiConnectionManager::WaitForConnectionUntilTimeout() {
    unsigned long t = millis();
    while ((millis() - t) < STA_TIMEOUT && WiFi.status() != WL_CONNECTED) {
        WiFiConnectionManager::task();
        yield();
    }
}

void WiFiConnectionManager::handleReconnecting() {
    // This state exists for clarity but flows back to STA_CONNECTING
    // handleSTAConnecting will manage the timeout and transitions
}

// ============================================================================
// EVENT & STATE TRANSITION
// ============================================================================

void WiFiConnectionManager::transitionToState(State newState) {
    if (newState == currentState) {
        return;
    }

    State oldState = currentState;
    currentState = newState;
    stateChangeTime = millis();

    Serial.print(F("[WiFiMgr] State transition: "));
    
    switch (oldState) {
        case State::IDLE:              Serial.print(F("IDLE")); break;
        case State::STA_CONNECTING:    Serial.print(F("STA_CONNECTING")); break;
        case State::STA_CONNECTED:     Serial.print(F("STA_CONNECTED")); break;
        case State::AP_RUNNING:        Serial.print(F("AP_RUNNING")); break;
        case State::RECONNECTING:      Serial.print(F("RECONNECTING")); break;
    }
    
    Serial.print(" → ");
    
    switch (newState) {
        case State::IDLE:              Serial.println(F("IDLE")); break;
        case State::STA_CONNECTING:    Serial.println(F("STA_CONNECTING")); break;
        case State::STA_CONNECTED:     Serial.println(F("STA_CONNECTED")); break;
        case State::AP_RUNNING:        Serial.println(F("AP_RUNNING")); break;
        case State::RECONNECTING:      Serial.println(F("RECONNECTING")); break;
    }
}

void WiFiConnectionManager::emitEvent(Event event, const char* info) {
    if (eventCallback == nullptr) {
        return;
    }

    switch (event) {
        case Event::STA_CONNECT_SUCCESS:
            Serial.print(F("[WiFiMgr] Event: STA_CONNECT_SUCCESS - ")); Serial.println(info);
            break;
        case Event::STA_CONNECT_FAILED:
            Serial.print(F("[WiFiMgr] Event: STA_CONNECT_FAILED - ")); Serial.println(info);
            break;
        case Event::AP_STARTED:
            Serial.print(F("[WiFiMgr] Event: AP_STARTED - ")); Serial.println(info);
            break;
        case Event::AP_TO_STA_SUCCESS:
            Serial.print(F("[WiFiMgr] Event: AP_TO_STA_SUCCESS - ")); Serial.println(info);
            break;
        case Event::RECONNECT_ATTEMPT:
            Serial.print(F("[WiFiMgr] Event: RECONNECT_ATTEMPT\n"));
            break;
    }

    eventCallback(event, info);
}

// ============================================================================
// RECONNECTION LOGIC WITH EXPONENTIAL BACKOFF
// ============================================================================

unsigned long WiFiConnectionManager::calculateBackoffDelay() {
    // Exponential backoff: 2^attempt, capped at MAX_BACKOFF_EXPONENT
    uint8_t exponent = (reconnectAttemptCount < MAX_BACKOFF_EXPONENT) 
                       ? reconnectAttemptCount 
                       : MAX_BACKOFF_EXPONENT;
    
    unsigned long baseDelay = 1000; // 1 second base
    unsigned long delay = baseDelay * (1UL << exponent); // 2^exponent
    
    // Cap at maximum interval
    return (delay < RECONNECT_INTERVAL) ? delay : RECONNECT_INTERVAL;
}

bool WiFiConnectionManager::shouldAttemptReconnect() {
    unsigned long now = millis();
    
    // Check if enough time has passed since last attempt
    if (now < nextReconnectAttempt) {
        return false;
    }

    reconnectAttemptCount++;
    unsigned long backoff = calculateBackoffDelay();
    nextReconnectAttempt = now + backoff;

    Serial.print(F("[WiFiMgr] Next reconnect attempt in")); Serial.print(backoff); Serial.print(F("ms (backoff level:")); Serial.print(reconnectAttemptCount); Serial.println(')'); 

    return true;
}

// ============================================================================
// PUBLIC STATE QUERIES
// ============================================================================

WiFiConnectionManager::State WiFiConnectionManager::getState() {
    return currentState;
}

bool WiFiConnectionManager::isConnected() {
    return currentState == State::STA_CONNECTED && WiFi.status() == WL_CONNECTED;
}

bool WiFiConnectionManager::isAPActive() {
    return currentState == State::AP_RUNNING;
}

String WiFiConnectionManager::getSSID() {
    if (currentState == State::STA_CONNECTED) {
        return WiFi.SSID();
    }
    return "";
}

IPAddress WiFiConnectionManager::getLocalIP() {
    if (currentState == State::STA_CONNECTED) {
        return WiFi.localIP();
    }
    return IPAddress(0, 0, 0, 0);
}

IPAddress WiFiConnectionManager::getAPIP() {
    if (currentState == State::AP_RUNNING) {
        return WiFi.softAPIP();
    }
    return IPAddress(0, 0, 0, 0);
}

int8_t WiFiConnectionManager::getSignalStrength() {
    if (currentState == State::STA_CONNECTED) {
        return WiFi.RSSI();
    }
    return 0;
}

unsigned long WiFiConnectionManager::getTimeSinceLastConnection() {
    if (lastSuccessfulConnection == 0) {
        return 0;
    }
    return (millis() - lastSuccessfulConnection) / 1000;
}

unsigned long WiFiConnectionManager::getNextReconnectAttemptTime() {
    return nextReconnectAttempt;
}

// ============================================================================
// MANUAL CONTROL
// ============================================================================

void WiFiConnectionManager::forceReconnect() {
    if (currentState == State::AP_RUNNING) {
        Serial.println(F("[WiFiMgr] Force reconnect: attempting WiFi from AP mode"));
        nextReconnectAttempt = 0; // Trigger immediate attempt
        reconnectAttemptCount = 0; // Reset backoff
    }
}

void WiFiConnectionManager::forceAPMode() {
    if (currentState != State::AP_RUNNING) {
        Serial.println(F("[WiFiMgr] Force AP mode"));
        WiFi.disconnect(false);
        WiFi.mode(WIFI_AP);
        WiFi.softAP(apSSID, apPassword);
        transitionToState(State::AP_RUNNING);
        nextReconnectAttempt = millis() + AP_TIMEOUT;
    }
}

// ============================================================================
// CONFIGURATION
// ============================================================================

void WiFiConnectionManager::setAPCredentials(const char* ssid, const char* password) {
    apSSID = ssid;
    apPassword = password;
}

void WiFiConnectionManager::setTimeouts(
    unsigned long staTimeout_ms,
    unsigned long apTimeout_ms,
    unsigned long reconnectInterval_ms
) {
    STA_TIMEOUT = staTimeout_ms;
    AP_TIMEOUT = apTimeout_ms;
    RECONNECT_INTERVAL = reconnectInterval_ms;
}

void WiFiConnectionManager::setEventCallback(EventCallback callback) {
    eventCallback = callback;
}
