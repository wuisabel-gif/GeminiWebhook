#!/usr/bin/env bash
# Host test for the GeminiWebhook library. Requires a C++17 compiler.
set -euo pipefail
cd "$(dirname "$0")/.."
out=$(mktemp "${TMPDIR:-/tmp}/gemini-webhook-test.XXXXXX")
trap 'rm -f "$out"' EXIT
"${CXX:-c++}" -std=c++17 -Wall -Itest -Isrc test/test_gemini_webhook.cpp -o "$out"
"$out"
