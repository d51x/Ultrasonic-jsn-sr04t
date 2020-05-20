#ifdef JSNSR04T_SOFTWARE_UART
  #include <SoftwareSerial.h>
#endif

#include "jsn-sr04t.h"


void JsnSr04t::begin(byte rx_pin, byte tx_pin){
        _rx = rx_pin;
          _tx = tx_pin;
          _distance = 0;

  #ifdef JSNSR04T_SOFTWARE_UART
    serial =  new SoftwareSerial(_rx, _tx, false);  
    serial->begin(9600);
  #else
    Serial.begin(9600);
  #endif

  delay(2000);
}

void JsnSr04t::write(){
  
  #ifdef JSNSR04T_SOFTWARE_UART
    serial->flush();
    serial->write( 0x55 );
  #else
    Serial.flush();
    Serial.write( 0x55 );
  #endif
  delay(100);
}

bool JsnSr04t::verify(char *data, byte sz){
    if ( sz == 4 && data[0] == 0xFF) {
      int sum = (data[0] + data[1] + data[2]) & 0xFF;
      if ( sum == data[3] ) {
        return true;
      }
    }
    return false;
}

int JsnSr04t::read(){
  int res = 0;
  if ( 
    #ifdef JSNSR04T_SOFTWARE_UART
      serial->available() 
    #else
      Serial.available()
    #endif
    > 0 ) 
  {
    char data[4];
    int readed = 0;
     
    #ifdef JSNSR04T_SOFTWARE_UART 
      readed = serial->readBytes(data, 4); 
    #else 
      readed = Serial.readBytes(data, 4); 
    #endif

    if ( verify(&data[0], readed) ) {
      res = ( ((int)data[1] << 8 ) + data[2]);
    }
    /*
    if ( readed == 4 && data[0] == 0xFF) {
      int sum = (data[0] + data[1] + data[2]) & 0xFF;
      if ( sum == data[3] ) {
        res = ( ((int)data[1] << 8 ) + data[2]);
      }
    }
    */
  }
  return res;
}

int JsnSr04t::getDistance(){
  _distance = 0;
  write();
  _distance = read();
  return _distance;  
}
