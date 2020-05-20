#ifndef JSN_SR04T_H
#define JSN_SR04T_H

#define JSNSR04T_SOFTWARE_UART

  #include <Arduino.h>

  #ifdef JSNSR04T_SOFTWARE_UART
    #include <SoftwareSerial.h>
  #endif

class JsnSr04t {
  public:


    void begin(byte rx_pin, byte tx_pin);
    int getDistance();
    
  private:
    byte _rx;
    byte _tx;
    int _distance;

    #ifdef JSNSR04T_SOFTWARE_UART
      SoftwareSerial *serial;
    #endif
    
    void write();
    int read();

    bool verify(char *data, byte sz);
};

#endif
