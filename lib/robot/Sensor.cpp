// #include "Sensor.hpp"

// namespace {

// constexpr char ENCRYPTION_KEY[] = "encryptionkey16";

// }  // namespace

// Sensor::Sensor()
//     : _comm(DisplaySensorProtocol::SENSOR_NODE_ID,
//             DisplaySensorProtocol::RF_FREQUENCY_MHZ),
//       _lastSendAtMs(0) {}

// void Sensor::begin_sen66() {
//     Wire.begin();
//     _sen66.begin(Wire, SEN66_I2C_ADDR_6B);
//     if (!startMeasurement()) {
//         Serial.println("Sensor measurement startup failed");
//     }
// }

// void Sensor::update_sen66() {

//     DisplaySensorProtocol::TelemetryFrame frame;
//     if (!readTelemetry(frame)) {
//         Serial.println("Sensor read skipped");
//         _lastSendAtMs = now;
//         return;
//     }

//     if (!sendTelemetry(frame)) {
//         Serial.println("Sensor send failed");
//     }
//     _lastSendAtMs = now;
// }

// bool Sensor::startMeasurement() {
//     int16_t error = _sen66.deviceReset();
//     if (error != 0) {
//         printError("SEN66 reset", error);
//         return false;
//     }

//     delay(1200);

//     error = _sen66.startContinuousMeasurement();
//     if (error != 0) {
//         printError("SEN66 start", error);
//         return false;
//     }

//     return true;
// }

// bool Sensor::readTelemetry(DisplaySensorProtocol::TelemetryFrame& frame) {
//     float pm1 = 0.0f;
//     float pm25 = 0.0f;
//     float pm4 = 0.0f;
//     float pm10 = 0.0f;
//     float humidity = 0.0f;
//     float temperature = 0.0f;
//     float voc = 0.0f;
//     float nox = 0.0f;
//     uint16_t co2 = 0;

//     int16_t error = _sen66.readMeasuredValues(
//         pm1, pm25, pm4, pm10, humidity, temperature, voc, nox, co2);
//     if (error != 0) {
//         printError("SEN66 read", error);
//         return false;
//     }

//     frame.temperature_centi_c = static_cast<int16_t>(temperature * 100.0f);
//     frame.humidity_centi_percent =
//         static_cast<uint16_t>(humidity * 100.0f);
//     frame.pm25_tenths_ug_m3 = static_cast<uint16_t>(pm25 * 10.0f);
//     frame.voc_index_tenths = static_cast<uint16_t>(voc * 10.0f);
//     frame.nox_index_tenths = static_cast<uint16_t>(nox * 10.0f);
//     frame.co2_ppm = co2;
//     return true;
// }

// bool Sensor::sendTelemetry(
//     const DisplaySensorProtocol::TelemetryFrame& frame) {
//     char payload[50];
//     if (!DisplaySensorProtocol::encodeTelemetry(frame, payload, sizeof(payload))) {
//         Serial.println("Sensor payload encode failed");
//         return false;
//     }

//     return _comm.send(DisplaySensorProtocol::DISPLAY_NODE_ID,
//                       DisplaySensorProtocol::SENSOR_TELEMETRY_COMMAND,
//                       payload);
// }

// void Sensor::printError(const char* label, int16_t error) {
//     char errorMessage[64];
//     errorToString(error, errorMessage, sizeof(errorMessage));
//     Serial.print(label);
//     Serial.print(": ");
//     Serial.println(errorMessage);
// }
