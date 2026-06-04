#include "Display.hpp"

namespace {

constexpr char ENCRYPTION_KEY[] = "encryptionkey16";

Display* g_displayInstance = nullptr;

void onReceive(RF69_Packet& packet) {
    if (g_displayInstance != nullptr) {
        g_displayInstance->handlePacket(packet);
    }
}

}  // namespace

Display::Display()
    : _display(kScreenWidth, kScreenHeight, &Wire),
      _comm(DisplaySensorProtocol::DISPLAY_NODE_ID,
            DisplaySensorProtocol::RF_FREQUENCY_MHZ),
      _hasTelemetry(false),
      _lastPacketAtMs(0),
      _packetCount(0) {}

void Display::begin() {
    Serial.begin(115200);
    Wire.begin();

    if (!_display.begin(kOledAddress, true)) {
        Serial.println("Display init failed");
        while (true) {
            delay(10);
        }
    }

    _display.setRotation(1);
    _display.setTextSize(1);
    _display.setTextColor(SH110X_WHITE);
    _display.clearDisplay();
    _display.setCursor(0, 0);
    _display.println("Display booting");
    _display.display();

    _comm.enable_debug(true);
    _comm.set_receive_handler(&onReceive);
    g_displayInstance = this;

    if (!_comm.begin(nullptr, ENCRYPTION_KEY)) {
        Serial.println("Display radio init failed");
    }

    drawWaitingScreen();
}

void Display::update() {
    _comm.update();

    const bool linkTimedOut =
        _hasTelemetry && (millis() - _lastPacketAtMs > kSignalTimeoutMs);
    if (linkTimedOut) {
        _hasTelemetry = false;
        drawWaitingScreen();
    }
}

void Display::handlePacket(RF69_Packet& packet) {
    if (packet.command != DisplaySensorProtocol::SENSOR_TELEMETRY_COMMAND) {
        return;
    }

    DisplaySensorProtocol::TelemetryFrame frame;
    if (!DisplaySensorProtocol::decodeTelemetry(packet.payload, frame)) {
        Serial.println("Display telemetry parse failed");
        return;
    }

    _telemetry = frame;
    _hasTelemetry = true;
    _lastPacketAtMs = millis();
    _packetCount++;
    Serial.print("Display packet #");
    Serial.print(_packetCount);
    Serial.print(" payload: ");
    Serial.println(packet.payload);
    drawTelemetryScreen();
}

void Display::drawWaitingScreen() {
    _display.clearDisplay();
    _display.setCursor(0, 0);
    _display.println("Waiting for RF");
    _display.println("sensor data...");
    _display.display();
}

void Display::drawTelemetryScreen() {
    _display.clearDisplay();
    _display.setCursor(0, 0);
    _display.println("Sensor Telemetry");
    _display.print("Pkt: ");
    _display.println(_packetCount);
    _display.print("Temp: ");
    _display.println(_telemetry.temperature_centi_c / 100.0f, 1);
    _display.print("Hum : ");
    _display.println(_telemetry.humidity_centi_percent / 100.0f, 1);
    _display.print("PM2.5:");
    _display.println(_telemetry.pm25_tenths_ug_m3 / 10.0f, 1);
    _display.print("VOC : ");
    _display.println(_telemetry.voc_index_tenths / 10.0f, 1);
    _display.print("NOx : ");
    _display.println(_telemetry.nox_index_tenths / 10.0f, 1);
    _display.print("CO2 : ");
    _display.println(_telemetry.co2_ppm);
    _display.display();
}
