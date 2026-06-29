#include "Joystick.hpp"
#include "ProtocolLayer.hpp"
#include "Arduino.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

Adafruit_SH1107 display(64, 128, &Wire, -1); // OLED

namespace {
constexpr uint8_t CONTROLLER_NODE_ID = 10;
constexpr uint8_t ROBOT_NODE_ID = 20;
constexpr float RF_FREQUENCY_MHZ = 868.0f;
constexpr char ENCRYPTION_KEY[] = "encryptionkey16";
constexpr unsigned long SEND_INTERVAL_MS = 20;

RF69_Comm comm(CONTROLLER_NODE_ID, RF_FREQUENCY_MHZ);
ProtocolLayer protocol(comm, ROBOT_NODE_ID);
Joystick robot_joy(A3, A2, false);

int lastThrottleDuty = -1;
int lastSteeringDuty = -1;
unsigned long lastSendTime = 0;

void sendControlValues(uint8_t throttleDuty, uint8_t steeringDuty) {
    protocol.sendThrottle(throttleDuty);
    protocol.sendSteering(steeringDuty);
    Serial.print("TX throttle=");
    Serial.print(throttleDuty);
    Serial.print(" steering=");
    Serial.println(steeringDuty);
}
}

void setup() {
    Serial.begin(115200);
    while (!Serial) {}
    Wire.begin();
    display.begin(0x3C, true);
    display.setRotation(1);
    display.clearDisplay();
    display.display();
    robot_joy.init_joystick();

    if (!comm.begin(nullptr, ENCRYPTION_KEY)) {
        Serial.println("Controller radio init failed");
        return;
    }
    protocol.setRemoteNodeId(ROBOT_NODE_ID);
    // Sensor display callback
     protocol.setSensorCallback([](const sen66_packet& data) {
        display.clearDisplay();
        display.setTextSize(1);
        display.setTextColor(SH110X_WHITE);
        display.setCursor(0, 0);

        display.print("Temp: "); display.print(data.temp, 1); display.println("C");
        display.print("RH:   "); display.print(data.rh, 1);  display.println("%");
        display.print("VOC:  "); display.println(data.voc, 1);
        display.print("NOx:  "); display.println(data.nox, 1);
        display.print("CO2:  "); display.print((int)data.co2_hcho); display.println("ppm");
        display.print("PM2.5:"); display.print(data.pm2_5, 1); display.println("ug/m3");

        display.display();
    });


    Serial.println("Controller radio started");
}

void loop() {
    int x = 0;
    int y = 0;
    robot_joy.update_joystick(x, y);

    const uint8_t steeringDuty = x;
    const uint8_t throttleDuty = y;

    const unsigned long now = millis();
    if (now - lastSendTime >= SEND_INTERVAL_MS) {
        // if (steeringDuty != lastSteeringDuty || throttleDuty != lastThrottleDuty) {
        sendControlValues(throttleDuty, steeringDuty);
        lastSteeringDuty = steeringDuty;
        lastThrottleDuty = throttleDuty;
        // }
        lastSendTime = now;
    }
}
