/*
 * mkrwan_04_gps_tracker.ino - GPS Tracker example code.
 * Copyright (c) 2018 Peter Recktenwald 
 * MIT License
 *
 * Credits:
 *    MKRWAN-1300 examples
 *    Copyright (c) 2018 Gonzalo Casas
 *    MIT License
 *    https://github.com/gonzalocasas/arduino-mkr-wan-1300 
 * GPS packet format and TTN Mapper integration by:
 *    Copyright (c) 2016 JP Meijers 
 *    Apache License, Version 2.0, http://www.apache.org/licenses/LICENSE-2.0
 *    https://github.com/jpmeijers/RN2483-Arduino-Library
 */
// MKRWAN - Version: Latest 
#include <MKRWAN.h>
#include <CayenneLPP.h>
#include <TinyGPS++.h>
#include "arduino_secrets.h" 

#define loraSerial Serial2
#define gpsSerial Serial1
#define debugSerial Serial

// LoRaWAN
// Select your region (AS923, AU915, EU868, KR920, IN865, US915, US915_HYBRID)
_lora_band region = EU868;
LoRaModem modem(loraSerial);

CayenneLPP lpp(51);

// GPS
TinyGPSPlus gps;

#define PMTK_SET_NMEA_UPDATE_05HZ  "$PMTK220,2000*1C"
#define PMTK_SET_NMEA_UPDATE_1HZ  "$PMTK220,1000*1F"
#define PMTK_SET_NMEA_OUTPUT_RMCGGA "$PMTK314,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*28"


void setup()
{
  debugSerial.begin(9600);
  gpsSerial.begin(9600);
  

  // Wait a maximum of 10s for Serial Monitor
  while (!debugSerial && millis() < 10000)
    ;

 // Begin LoRa modem
  if (!modem.begin(region)) {
    debugSerial.println(F("Failed to start module"));
  };

  debugSerial.print(F("Your device EUI is: "));
  debugSerial.println(modem.deviceEUI());

  int connected = modem.joinOTAA(appEui, appKey);
  if (!connected) {
    debugSerial.println(F("Something went wrong; are you indoor? Move near a window and retry"));
  }

  debugSerial.println(F("Successfully joined the network!"));
 
  debugSerial.println(F("Enabling ADR and setting low spreading factor"));
  modem.setADR(true);
  modem.dataRate(5);
  
  debugSerial.println(F("GPS serial init"));
//  gpsSerial.println(F(PMTK_SET_NMEA_OUTPUT_RMCGGA));
  gpsSerial.println(F(PMTK_SET_NMEA_UPDATE_1HZ));   // 1 Hz update rate
  debugSerial.println(F("GPS serial ready"));
}

void loop()
{
  while (gpsSerial.available() > 0) {
    if (gps.encode(gpsSerial.read())) {
      // For debugging
      displayGpsInfo();
      sendCoords();

      // Delay between updates
      delay(2000);
    }
  }

  if (millis() > 5000 && gps.charsProcessed() < 10) {
    debugSerial.println(F("No GPS detected: check wiring."));
  }
}

void sendCoords(void)
{

  lpp.reset();
  lpp.addGPS(1, gps.location.lat(), gps.location.lng(), gps.altitude.meters());

  modem.beginPacket();
  modem.write(lpp.getBuffer(), lpp.getSize());
  int err = modem.endPacket(false);
  
  if (err > 0) {
    debugSerial.println("Big success!");
  } else {
      debugSerial.println("Error");
  }
  debugSerial.println("TX done");

}


void displayGpsInfo()
{
  debugSerial.print(F("Location: ")); 
  if (gps.location.isValid())
  {
    debugSerial.print(gps.location.lat(), 6);
    debugSerial.print(F(","));
    debugSerial.print(gps.location.lng(), 6);
  }
  else
  {
    debugSerial.print(F("INVALID"));
  }

  debugSerial.print(F("  Date/Time: "));
  if (gps.date.isValid())
  {
    debugSerial.print(gps.date.month());
    debugSerial.print(F("/"));
    debugSerial.print(gps.date.day());
    debugSerial.print(F("/"));
    debugSerial.print(gps.date.year());
  }
  else
  {
    debugSerial.print(F("INVALID"));
  }

  debugSerial.print(F(" "));
  if (gps.time.isValid())
  {
    if (gps.time.hour() < 10) debugSerial.print(F("0"));
    debugSerial.print(gps.time.hour());
    debugSerial.print(F(":"));
    if (gps.time.minute() < 10) debugSerial.print(F("0"));
    debugSerial.print(gps.time.minute());
    debugSerial.print(F(":"));
    if (gps.time.second() < 10) debugSerial.print(F("0"));
    debugSerial.print(gps.time.second());
    debugSerial.print(F("."));
    if (gps.time.centisecond() < 10) debugSerial.print(F("0"));
    debugSerial.print(gps.time.centisecond());
  }
  else
  {
    debugSerial.print(F("INVALID"));
  }

  debugSerial.println();
}


