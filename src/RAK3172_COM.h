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

#endif
