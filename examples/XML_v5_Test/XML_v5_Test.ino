/*  XML_v5_Test.ino
 *  Author: Alex St. Clair
 *  Created: July 2019
 */
 
#include <XMLReader_v5.h>
#include <XMLWriter_v5.h>

XMLReader reader(&Serial, RACHUTS);
XMLWriter writer(&Serial, RACHUTS);
TCParseStatus_t tc_status = NO_TCs;

void setup()
{
  Serial.begin(115200);
  //Serial1.begin(115200);
  delay(3000);
}

void loop()
{
  delay(500);
  while (reader.GetNewMessage()) {
    Serial.print("\nReceived message: "); Serial.println(reader.zephyr_message);
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
      Serial.print("Num TC's:  "); Serial.println(reader.num_tcs);
      Serial.print("TC buffer: "); Serial.println(reader.tc_buffer);
      tc_status = reader.GetTelecommand();
      while (NO_TCs != tc_status) {
        if (TC_ERROR == tc_status) {
          Serial.println("TC Error");
        } else if (READ_TC == tc_status) {
          // trigger messages to send
          Serial.print("TC: "); Serial.println(reader.zephyr_tc);
          switch (reader.zephyr_tc) {
          case 150:
            writer.IMR();
            break;
          case 151:
            writer.S();
            break;
          case 152:
            writer.RA();
            break;
          case 153:
            writer.IMAck(true);
            break;
          case 154:
            writer.TCAck(false);
            break;
          case 155:
            writer.addTm((const uint8_t *) "test binary string", 18);
            writer.TM();
            break;
          default:
            break;
          }
        }
        tc_status = reader.GetTelecommand();
      }
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

