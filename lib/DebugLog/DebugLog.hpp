#pragma once

#include <Arduino.h>
#include <stdio.h>
#include <string.h>

extern char debugLineBuffer[];
extern size_t debugLineLength;
#ifndef DEBUG_LINE_BUFFER_SIZE
#define DEBUG_LINE_BUFFER_SIZE 512
#endif
static constexpr size_t DEBUG_LINE_BUFFER_SIZE_VALUE = DEBUG_LINE_BUFFER_SIZE;

namespace DebugLog {
inline void flush();

inline void reset() {
    debugLineBuffer[0] = '\0';
    debugLineLength = 0;
}

inline void appendText(const char *text) {
    if (text == nullptr) {
        return;
    }

    const char *cursor = text;
    while (*cursor != '\0') {
        if (debugLineLength >= DEBUG_LINE_BUFFER_SIZE_VALUE - 1) {
            flush();
        }

        size_t available = DEBUG_LINE_BUFFER_SIZE_VALUE - 1 - debugLineLength;
        if (available == 0) {
            continue;
        }

        size_t chunkLength = 0;
        while (chunkLength < available && cursor[chunkLength] != '\0') {
            ++chunkLength;
        }

        memcpy(debugLineBuffer + debugLineLength, cursor, chunkLength);
        debugLineLength += chunkLength;
        debugLineBuffer[debugLineLength] = '\0';
        cursor += chunkLength;
    }
}

inline void appendSeparator() {
    if (debugLineLength > 0 && debugLineBuffer[debugLineLength - 1] != ' ') {
        appendText(" ");
    }
}

inline void appendField(const char *label, int value) {
    char tmp[32];
    snprintf(tmp, sizeof(tmp), "%s=%d", label, value);
    appendSeparator();
    appendText(tmp);
}

inline void appendField(const char *label, unsigned long value) {
    char tmp[32];
    snprintf(tmp, sizeof(tmp), "%s=%lu", label, value);
    appendSeparator();
    appendText(tmp);
}

inline void formatFloat(float value, char *buf, size_t size) {
    if (size == 0) {
        return;
    }

    if (value < 0.0f) {
        if (size < 2) {
            buf[0] = '\0';
            return;
        }
        *buf++ = '-';
        size -= 1;
        value = -value;
    }

    long integerPart = static_cast<long>(value);
    float remainder = value - static_cast<float>(integerPart);
    long fraction = static_cast<long>(remainder * 100.0f + 0.5f);
    if (fraction >= 100) {
        integerPart += 1;
        fraction = 0;
    }

    snprintf(buf, size, "%ld.%02ld", integerPart, fraction);
}

inline void appendField(const char *label, float value) {
    char tmp[32];
    char valbuf[16];
    formatFloat(value, valbuf, sizeof(valbuf));
    snprintf(tmp, sizeof(tmp), "%s=%s", label, valbuf);
    appendSeparator();
    appendText(tmp);
}

inline void appendField(const char *label, const char *value) {
    char tmp[48];
    snprintf(tmp, sizeof(tmp), "%s=%s", label, value ? value : "null");
    appendSeparator();
    appendText(tmp);
}

inline void flush() {
    if (debugLineLength > 0) {
        Serial.println(debugLineBuffer);
        reset();
    }
}
}