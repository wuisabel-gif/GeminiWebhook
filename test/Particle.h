// Host-only stand-in for the Particle API, just enough to test gemini_webhook.h.
#pragma once
#include <string>
#include <functional>

class String : public std::string {
public:
    using std::string::string;
    String() = default;
    String(const std::string& s) : std::string(s) {}
};
struct Logger {
    template<class... T> void warn(const char*, T...) {}
    template<class... T> void error(const char*, T...) {}
} Log;
unsigned long clockMs = 0;
unsigned long millis() { return clockMs; }
constexpr int MY_DEVICES = 0;
struct Cloud {
    bool publishOK = true;
    int publishes = 0;
    String event, payload, subscribedTo;
    std::function<void(const char*, const char*)> handler;
    bool publish(const String& name, const String& data) {
        ++publishes; event = name; payload = data; return publishOK;
    }
    void subscribe(const String& name, void (*f)(const char*, const char*), int) {
        subscribedTo = name; handler = f;
    }
} Particle;
