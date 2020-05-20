/*
  ESP8266 BlinkWithoutDelay by Simon Peter
  Blink the blue LED on the ESP-01 module
  Based on the Arduino Blink without Delay example
  This example code is in the public domain

  The blue LED on the ESP-01 module is connected to GPIO1
  (which is also the TXD pin; so we cannot use Serial.print() at the same time)

  Note that this sketch uses LED_BUILTIN to find the pin with the internal LED
*/


#include <SoftwareSerial.h>

SoftwareSerial serial2(2, 0);

int ledState = LOW;

unsigned long currentMillis = 0;
const long interval = 1000;

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(9600);
  serial2.begin(9600, SWSERIAL_8N1, 2, 0, false);
  delay(1000);
}

void loop() {
  if (millis() - currentMillis >= interval) {
    currentMillis = millis();    
    Serial.print("time: ");
    Serial.println( currentMillis );

    int cnt = serial2.available();
    Serial.print("bytes available: ");
    Serial.println( cnt );
    
    while ( serial2.available() > 0 ) {
      byte b = serial2.read();
      if ( b != 0xFF ) continue;
      char data[3];
      int avail = serial2.read(data, 3);
      if ( avail <= 0 ) continue;
      byte sum = (0xFF + data[0] + data[1]) & 0xFF;
      if ( sum != data[2] ) {
        Serial.print("ERROR: checksum wrong...");
        Serial.print("reseived 0x"); Serial.print( data[2], HEX);
        Serial.print(", calculated 0x"); Serial.print( sum, HEX);
        Serial.println();
        //serial2.flush();
        continue;
      }
      int dist = data[0] * 256 + data[1];
      Serial.print("Distance: "); Serial.println( dist );
      //serial2.flush();
    }
  }
}
