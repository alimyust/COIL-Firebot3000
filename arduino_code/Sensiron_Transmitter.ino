//sender

#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <RFM69.h>
#include <SensirionI2cSen66.h>

#define NETWORKID 100
#define NODEID_SENDER 1
#define NODEID_DISPLAY 2

#define RFM69_CS   8
#define RFM69_INT  3
#define RFM69_RST  4

#define FREQUENCY RF69_868MHZ

RFM69 radio(RFM69_CS, RFM69_INT, true);

SensirionI2cSen66 sensor;

struct Sen66Packet {
  float pm1p0;
  float pm2p5;
  float pm4p0;
  float pm10p0;
  float humidity;
  float temperature;
  float vocIndex;
  float noxIndex;
  uint16_t co2;
  uint32_t counter;
};

Sen66Packet data;

void setup() {
  Serial.begin(115200);

  Wire.begin();

  sensor.begin(Wire, SEN66_I2C_ADDR_6B);

  sensor.deviceReset();
  delay(1200);
  sensor.startContinuousMeasurement();
  delay(1500);

  pinMode(RFM69_RST, OUTPUT);

  digitalWrite(RFM69_RST, LOW);
  delay(10);
  digitalWrite(RFM69_RST, HIGH);
  delay(10);
  digitalWrite(RFM69_RST, LOW);
  delay(10);

  radio.initialize(FREQUENCY, NODEID_SENDER, NETWORKID);
  radio.encrypt("1234567890123456");

  Serial.println("Sender gestartet");
}

void loop() {

  float pm1p0, pm2p5, pm4p0, pm10p0;
  float humidity, temperature;
  float vocIndex, noxIndex;
  uint16_t co2;

  int16_t error = sensor.readMeasuredValues(
      pm1p0,
      pm2p5,
      pm4p0,
      pm10p0,
      humidity,
      temperature,
      vocIndex,
      noxIndex,
      co2);

  if (error == 0) {

    data.pm1p0 = pm1p0;
    data.pm2p5 = pm2p5;
    data.pm4p0 = pm4p0;
    data.pm10p0 = pm10p0;

    data.humidity = humidity;
    data.temperature = temperature;

    data.vocIndex = vocIndex;
    data.noxIndex = noxIndex;

    data.co2 = co2;

    data.counter++;

//test
Serial.println("=================================");
Serial.print("Packet #: ");
Serial.println(data.counter);

Serial.print("PM1.0     : ");
Serial.print(data.pm1p0, 1);
Serial.println(" ug/m3");

Serial.print("PM2.5     : ");
Serial.print(data.pm2p5, 1);
Serial.println(" ug/m3");

Serial.print("PM4.0     : ");
Serial.print(data.pm4p0, 1);
Serial.println(" ug/m3");

Serial.print("PM10      : ");
Serial.print(data.pm10p0, 1);
Serial.println(" ug/m3");

Serial.print("Temp      : ");
Serial.print(data.temperature, 1);
Serial.println(" C");

Serial.print("Humidity  : ");
Serial.print(data.humidity, 1);
Serial.println(" %");

Serial.print("VOC Index : ");
Serial.println(data.vocIndex, 1);

Serial.print("NOx Index : ");
Serial.println(data.noxIndex, 1);

Serial.print("CO2       : ");
Serial.print(data.co2);
Serial.println(" ppm");


    radio.send(
      NODEID_DISPLAY,
      (const void*)&data,
      sizeof(data)
    );

    Serial.println("Daten gesendet");
  }

  delay(1000);
}