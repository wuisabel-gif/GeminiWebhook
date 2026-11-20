#pragma once
#include "Particle.h"

/*
  GeminiWebhook
  Send text to an AI model through a Particle webhook.

  The library takes care of the parts that are the same in every project:
    - sending no more often than your free quota allows
    - knowing whether you are still waiting for a reply
    - giving up if no reply comes (timeout)
    - telling you when the webhook reports an error

  You write the parts that are different in every project:
    - the webhook (it holds the prompt and the API key)
    - what text to send
    - subscribing to the reply and unpacking its JSON

  See examples/ask-gemini for a complete sketch.
*/

// Defaults. Pass other values to geminiSetup() to change them.
#define GEMINI_MIN_GAP_MS 10000    // at most one request every 10 s (free-tier quota)
#define GEMINI_TIMEOUT_MS 20000    // no reply after 20 s = error

// You define this function in your sketch. It is called when a request
// fails: the webhook reported an error, nothing came back in time, or the
// publish did not go out. why says which.
void onGeminiError(String why);

// Call once in setup(). eventName must match the webhook's event name exactly.
void geminiSetup(const char* eventName,
                 unsigned long minGapMs = GEMINI_MIN_GAP_MS,
                 unsigned long timeoutMs = GEMINI_TIMEOUT_MS);

// Call every time through loop(). Reports a timeout if no reply came.
void geminiLoop();

// Send text to the webhook. Returns true if it went out, false if it was
// too soon after the last request or a reply is still pending.
bool geminiSend(String text);

// true when geminiSend() would send: nothing pending, and the gap has passed
bool geminiReady();

// true while a request is out and no reply has arrived yet
bool geminiWaiting();

// Call at the start of your reply handler. It stops the timeout. Returns
// false if no request was pending (a late or duplicate reply), so you can
// ignore it.
bool geminiGotReply();
