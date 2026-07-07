#include <unity.h>
#include "ControllerHandler.hpp"
#include "RobotHandler.hpp"

void test_controller_payload_roundtrip(void) {
    ProtocolCommands::ThrottlePayload decoded{};
    RadioComm::RF69_Packet packet{};
    packet.command = ProtocolCommands::CMD_THROTTLE;
    strncpy(packet.payload, "42", sizeof(packet.payload) - 1);
    packet.payload[sizeof(packet.payload) - 1] = '\0';

    TEST_ASSERT_TRUE(ProtocolCommands::deserializeThrottlePayload(packet, decoded));
    TEST_ASSERT_EQUAL_UINT8(42, decoded.duty);
}

void test_robot_payload_roundtrip(void) {
    ProtocolCommands::SteeringPayload decoded{};
    RadioComm::RF69_Packet packet{};
    packet.command = ProtocolCommands::CMD_STEERING;
    strncpy(packet.payload, "87", sizeof(packet.payload) - 1);
    packet.payload[sizeof(packet.payload) - 1] = '\0';

    TEST_ASSERT_TRUE(ProtocolCommands::deserializeSteeringPayload(packet, decoded));
    TEST_ASSERT_EQUAL_UINT8(87, decoded.duty);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_controller_payload_roundtrip);
    RUN_TEST(test_robot_payload_roundtrip);
    return UNITY_END();
}
