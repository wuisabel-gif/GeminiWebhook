/*
  ask-gemini: send a sensor reading to Gemini every minute and print the answer.

  Before you flash:
    1. Install the ArduinoJson library (Particle: Install Library).
    2. Create a webhook in the Particle Console with the event name "my_event".
       See this library's README for the fields. Your API key goes in the
       webhook, never in this file.
*/
#include "Particle.h"

#define ARDUINOJSON_ENABLE_ARDUINO_STRING 1
#include <ArduinoJson.h>

#include "GeminiWebhook.h"

SYSTEM_MODE(AUTOMATIC);
SYSTEM_THREAD(ENABLED);
SerialLogHandler logHandler(LOG_LEVEL_WARN);

unsigned long lastAskMs = 0;

// Runs when the webhook's reply arrives. data holds the model's JSON answer.
void myHandler(const char* event, const char* data) {
    if (!geminiGotReply()) return;               // stops the timeout; ignores late replies

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, data);
    if (error) {
        onGeminiError("Reply was not JSON");
        return;
    }
    String answer = doc["answer"].as<String>();  // use the keys from your own prompt
    Serial.println("Gemini says: " + answer);
}

// Runs when a request fails or times out.
void onGeminiError(String why) {
    Serial.println("Problem: " + why);
}

void setup() {
    Serial.begin(9600);
    geminiSetup("my_event");
    // Some webhooks put the device ID in front of the reply's name, so listen for both.
    Particle.subscribe("hook-response/my_event", myHandler, MY_DEVICES);
    Particle.subscribe(System.deviceID() + "/hook-response/my_event", myHandler, MY_DEVICES);
}

void loop() {
    geminiLoop();

    if (millis() - lastAskMs >= 60000 && geminiReady()) {
        lastAskMs = millis();
        int light = analogRead(A0);              // replace with your own sensor
        geminiSend("light=" + String(light));
    }
}
