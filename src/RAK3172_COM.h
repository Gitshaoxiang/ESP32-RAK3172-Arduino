/*
 PubSubClient.h - A simple client for MQTT.
  Nick O'Leary
  http://knolleary.net
*/

#ifndef _RAK3172_COMMON_H
#define _RAK3172_COMMON_H

#include <Arduino.h>

String encodeMsg(String str);
String decodeMsg(String hexEncoded);
String bytes2hex(const uint8_t *buf, size_t size);
long hex2bin(String hex);
void hex2bytes(String hexEncoded, uint8_t *buf, size_t size);

class RAK3172 {
   protected:
    HardwareSerial *_serial;
    int _tx_pin;
    int _rx_pin;
    SemaphoreHandle_t _serial_mutex;

   public:
    bool init(HardwareSerial *serial = &Serial2, int rx = 16, int tx = 17,
              int baudrate = 115200);
    bool sendCommand(String cmd);
    size_t debug(String cmd);
};

#endif
