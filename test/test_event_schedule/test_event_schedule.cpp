#include <Arduino.h>
#include <unity.h>
#include "radio.h"
#include "ProtocolLayer.hpp"

class TestRobotHandler : public ProtocolLayer::ProtocolHandler {
public:
    bool throttle_called = false;
    bool steering_called = false;
    bool heartbeat_called = false;
    uint8_t throttle_value = 0;
    uint8_t steering_value = 0;

    void onThrottle(uint8_t duty) override {
        throttle_called = true;
        throttle_value = duty;
    }

    void onSteering(uint8_t duty) override {
        steering_called = true;
        steering_value = duty;
    }

    void onHeartbeat() override {
        heartbeat_called = true;
    }
};

static EventRadioComm comm(20, 868.0f);
static ProtocolLayer protocol(comm, 10);
static TestRobotHandler test_handler;

void test_event_radio_packet_dispatch() {
    comm.setTestMode(true);
    protocol.setHandler(&test_handler);

    RF69_Packet packet = {};
    packet.sender_id = 10;
    packet.receiver_id = 20;
    packet.command = ProtocolLayer::THROTTLE;
    strcpy(packet.payload, "128");

    comm.injectPacket(packet);
    protocol.process();

    TEST_ASSERT_TRUE(test_handler.throttle_called);
    TEST_ASSERT_EQUAL(128, test_handler.throttle_value);
}

void test_event_telemetry_tick_dispatch() {
    comm.queueTelemetryTick();
    protocol.process();

    TEST_ASSERT_TRUE(test_handler.heartbeat_called);
}

void setup() {
    delay(1000);
    UNITY_BEGIN();

    RUN_TEST(test_event_radio_packet_dispatch);
    RUN_TEST(test_event_telemetry_tick_dispatch);

    UNITY_END();
}

void loop() {
}
