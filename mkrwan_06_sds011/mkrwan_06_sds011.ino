/*
   mkrwan_06_sds011.ino - SDS011 Feinstaubsensor example code.
   Copyright (c) 2018 Peter Recktenwald
   MIT License

   Credits:
   MKRWAN-1300 examples
      Copyright (c) 2018 Gonzalo Casas
      MIT License
      https://github.com/gonzalocasas/arduino-mkr-wan-1300
   TTN Mapper integration by:
      Copyright (c) 2016 JP Meijers
      Apache License, Version 2.0, http://www.apache.org/licenses/LICENSE-2.0
      https://github.com/jpmeijers/RN2483-Arduino-Library
*/
#include <MKRWAN.h>
#include <SDS011.h>
#include "arduino_secrets.h"

#define debugSerial Serial
#define loraSerial Serial2

// LoRaWAN
// Select your region (AS923, AU915, EU868, KR920, IN865, US915, US915_HYBRID)
_lora_band region = EU868;
LoRaModem modem(loraSerial);

// SDS011
SDS011 sds;
int pm10, pm25;
int error;

// TTN Mapper stuff
unsigned long last_update = 0;
uint8_t txBuffer[6];

// Flashy stuff
#define VERY_FAST 50
#define FAST 200
#define SLOW 500
#define FOREVER -1

void flash(int times, unsigned int speed) {
  while (times == -1 || times--) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(speed);
    digitalWrite(LED_BUILTIN, LOW);
    delay(speed);
  }
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  debugSerial.begin(9600);

  flash(10, VERY_FAST);

  // Wait a maximum of 5s for Serial Monitor serial
  while (!debugSerial && millis() < 5000);
  debugSerial.println(F("Starting up..."));
  flash(2, SLOW);

  // Begin LoRa modem
  if (!modem.begin(region)) {
    debugSerial.println(F("Failed to start module"));
    flash(FOREVER, VERY_FAST);
  };

  debugSerial.print(F("Your device EUI is: "));
  debugSerial.println(modem.deviceEUI());
  flash(2, SLOW);

  int connected = modem.joinOTAA(appEui, appKey);
  if (!connected) {
    debugSerial.println(F("Something went wrong; are you indoor? Move near a window and retry"));
    flash(FOREVER, VERY_FAST);
  }
  debugSerial.println(F("Successfully joined the network!"));

  debugSerial.println(F("Enabling ADR and setting low spreading factor"));
  modem.setADR(true);
  modem.dataRate(5);

  sds.begin(&Serial1);
  //  Serial.println(sds.queryFirmwareVersion().toString()); // prints firmware version
  //  Serial.println(sds.setQueryReportingMode().toString()); // ensures sensor is in 'query' reporting mode

  flash(2, SLOW);
}

void loop() {
  error = sds.readInt(&pm25, &pm10);
  if (!error) {
    sendData();
    Serial.println("P2.5: " + String(pm25));
    Serial.println("P10:  " + String(pm10));
  }
  delay(60000); // wait 1 minute
}

void buildPacket() {

  txBuffer[0] = ( pm25 >> 16 ) & 0xFF;
  txBuffer[1] = ( pm25 >> 8 ) & 0xFF;
  txBuffer[2] = pm25 & 0xFF;

  txBuffer[3] = ( pm10 >> 16 ) & 0xFF;
  txBuffer[4] = ( pm10 >> 8 ) & 0xFF;
  txBuffer[5] = pm10 & 0xFF;
}

void sendData() {
  buildPacket();

  modem.beginPacket();
  modem.write(txBuffer, sizeof(txBuffer));
  int err = modem.endPacket(false);

  if (err > 0) {
    debugSerial.println("Big success!");
    flash(3, FAST);
  } else {
    debugSerial.println("Error");
    flash(10, VERY_FAST);
  }
  debugSerial.println("TX done");

  delay(3000);
  digitalWrite(LED_BUILTIN, LOW);
}


