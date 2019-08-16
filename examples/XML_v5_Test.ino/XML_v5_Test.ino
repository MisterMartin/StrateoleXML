/*  XML_v5_Test.ino
 *  Author: Alex St. Clair
 *  Created: July 2019
 */
 
#include <XMLReader_v5.h>

XMLReader reader(&Serial, RACHUTS);

void setup()
{
  Serial.begin(115200);
  delay(3000);
}

void loop()
{
  delay(500);
  while (reader.GetNewMessage()) {
    Serial.print("Received message: "); Serial.println(reader.zephyr_message);
    switch (reader.zephyr_message) {
    case IM:
      Serial.print("Mode: "); Serial.println(reader.zephyr_mode);
      break;
    case SAck:
    case RAAck:
    case TMAck:
      Serial.print("Ackval: "); Serial.println(reader.zephyr_ack);
      break;
    case TC:
      Serial.print("TC length: "); Serial.println(reader.tc_length);
      break;
    case GPS:
      Serial.print("Date: "); Serial.print(reader.zephyr_gps.year);
      Serial.print('/'); Serial.print(reader.zephyr_gps.month);
      Serial.print('/'); Serial.println(reader.zephyr_gps.day);
      Serial.print("Time: "); Serial.print(reader.zephyr_gps.hour);
      Serial.print(':'); Serial.print(reader.zephyr_gps.minute);
      Serial.print(':'); Serial.println(reader.zephyr_gps.second);
      Serial.print("Lon: "); Serial.println(reader.zephyr_gps.longitude);
      Serial.print("Lat: "); Serial.println(reader.zephyr_gps.latitude);
      Serial.print("Alt: "); Serial.println(reader.zephyr_gps.altitude);
      Serial.print("SZA: "); Serial.println(reader.zephyr_gps.solar_zenith_angle);
      Serial.print("Qaulity: "); Serial.println(reader.zephyr_gps.quality);
      break;
    default:
      break;
    }
  }
}

