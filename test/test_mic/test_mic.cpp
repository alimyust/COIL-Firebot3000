
#include <mic.hpp>
#include <Arduino.h>

Microphone mic;

void setup() {
    Serial.begin(115200);
    while (!Serial) {
        delay(10);
    }
    Serial.println("Starting microphone test...");
    mic.begin();
}

void loop() {
    mic.update();
}