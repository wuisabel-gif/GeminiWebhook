#include "GeminiWebhook.h"

static String eventName;
static unsigned long minGapMs = GEMINI_MIN_GAP_MS;
static unsigned long timeoutMs = GEMINI_TIMEOUT_MS;
static bool waiting = false;
static bool hasSent = false;
static unsigned long lastSendMs = 0;

static void fail(String why) {
    waiting = false;
    Log.error("%s", why.c_str());
    onGeminiError(why);
}

// The webhook could not get an answer. data holds the reason, for example
// an HTTP error code or a message from Particle.
static void hookError(const char* event, const char* data) {
    if (waiting) fail(String("Webhook error: ") + (data ? data : ""));
}

void geminiSetup(const char* name, unsigned long gapMs, unsigned long toMs) {
    eventName = name;
    minGapMs = gapMs;
    timeoutMs = toMs;
    Particle.subscribe(String("hook-error/") + name, hookError, MY_DEVICES);
}

bool geminiWaiting() { return waiting; }

bool geminiReady() {
    return !waiting && (!hasSent || millis() - lastSendMs >= minGapMs);
}

bool geminiSend(String text) {
    if (!geminiReady()) {
        Log.warn("Too soon, request skipped");
        return false;
    }
    waiting = true;
    hasSent = true;
    lastSendMs = millis();
    if (!Particle.publish(eventName, text)) {
        fail("Could not publish");
        return false;
    }
    return true;
}

bool geminiGotReply() {
    if (!waiting) return false;
    waiting = false;
    return true;
}

void geminiLoop() {
    if (waiting && millis() - lastSendMs > timeoutMs) {
        fail("No reply from webhook (timeout)");
    }
}
