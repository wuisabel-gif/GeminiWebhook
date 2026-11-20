// Host test for the GeminiWebhook library. Run: test/run.sh
#include <cassert>
#include <cstdio>
#include "GeminiWebhook.h"
#include "../src/GeminiWebhook.cpp"   // one build, no linker step

int errors = 0;
String lastError;
void onGeminiError(String why) { ++errors; lastError = why; }

int main() {
    geminiSetup("my_event");
    assert(Particle.subscribedTo == "hook-error/my_event");

    // first send goes out right away, to the event name given in setup
    assert(geminiReady() && !geminiWaiting());
    assert(geminiSend("hello"));
    assert(Particle.publishes == 1 && Particle.event == "my_event" && Particle.payload == "hello");
    assert(geminiWaiting() && !geminiReady());

    // a second send while waiting is refused
    assert(!geminiSend("again") && Particle.publishes == 1);

    // reply arrives: accepted once, a duplicate is ignored
    assert(geminiGotReply() && !geminiWaiting());
    assert(!geminiGotReply());

    // still too soon until the gap has passed
    clockMs = GEMINI_MIN_GAP_MS - 1;
    assert(!geminiSend("too soon") && Particle.publishes == 1);
    clockMs = GEMINI_MIN_GAP_MS;
    assert(geminiSend("second") && Particle.publishes == 2);

    // no reply: nothing at the limit, error just after it, and only once
    clockMs += GEMINI_TIMEOUT_MS;
    geminiLoop();
    assert(errors == 0);
    clockMs += 1;
    geminiLoop();
    geminiLoop();
    assert(errors == 1 && !geminiWaiting());
    assert(!geminiGotReply());                  // a reply after the timeout is ignored

    // the webhook reports an error while we wait
    assert(geminiSend("third"));
    Particle.handler("hook-error/my_event/0", "error status 429");
    assert(errors == 2 && lastError == "Webhook error: error status 429" && !geminiWaiting());
    Particle.handler("hook-error/my_event/0", "stray");   // not waiting: ignored
    assert(errors == 2);

    // publish fails
    clockMs += GEMINI_MIN_GAP_MS;
    Particle.publishOK = false;
    assert(!geminiSend("fourth") && errors == 3 && !geminiWaiting());

    // custom limits passed to geminiSetup()
    geminiSetup("other_event", 1000, 2000);
    Particle.publishOK = true;
    clockMs += 1000;
    assert(geminiSend("fifth") && Particle.event == "other_event");
    clockMs += 2001;
    geminiLoop();
    assert(errors == 4);

    puts("GeminiWebhook: all checks passed");
}
