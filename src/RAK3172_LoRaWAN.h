#ifndef _RAK3172_LoRaWAN_H
#define _RAK3172_LoRaWAN_H

#include <Arduino.h>
#include "Stream.h"
#include <vector>
#include "RAK3172_LoRaWAN_DEF.h"
#include "RAK3172_COM.h"

typedef enum { OTAA = 0, ABP } LoRaWAN_Join_mode_t;
typedef enum { CLASS_A = 0, CLASS_B, CLASS_C } LoRaWAN_Dev_Class_t;

typedef struct {
    int rssi;
    int snr;
    int len;
    int port;
    char payload[500];
} LoRaWAN_frame_t;

class RAK3172_LoRaWAN : public RAK3172 {
   private:
    std::vector<LoRaWAN_frame_t> _frames;
    LoRaWAN_Dev_Class_t _class_mode;
    LoRaWAN_Join_mode_t _join_mode;
    bool _is_joined;
    bool _data_comfirm;

   public:
    bool init(HardwareSerial *serial = &Serial2, int rx = 16, int tx = 17,
              int baudrate = 115200);
    bool configBAND(String band, String channel_mask);
    bool configOTAA(String deveui, String appeui, String appkey);
    bool configABP(String devaddr, String nwkskey, String appskey);
    bool setMode(LoRaWAN_Dev_Class_t mode);
    bool join();
    size_t send(String data, int port = 1);
    size_t send(const uint8_t *buf, size_t size, int port = 1);
};

#endif
