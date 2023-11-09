#ifndef _RAK3172_LoRaWAN_H
#define _RAK3172_LoRaWAN_H

#include <Arduino.h>
#include "Stream.h"
#include <vector>
#include "RAK3172_LoRaWAN_DEF.h"
#include "RAK3172_COM.h"

typedef enum { OTAA = 0, ABP } LoRaWAN_Join_mode_t;
typedef enum { CLASS_A = 0, CLASS_B, CLASS_C } LoRaWAN_Dev_Class_t;
typedef enum {
    DIS_LINKCHECK = 0,
    ONCE_LINKCHECK,
    ALLWAYS_LINKCHECK
} LoRaWAN_LinkCheck_t;

typedef enum { ERROR = 0 } LoRaWAN_Error_t;

typedef struct {
    int rssi;
    int snr;
    int len;
    int port;
    bool unicast;
    char payload[500];
} LoRaWAN_frame_t;

class RAK3172_LoRaWAN : public RAK3172 {
   private:
    std::vector<LoRaWAN_frame_t> _frames;
    LoRaWAN_Dev_Class_t _class_mode;
    LoRaWAN_Join_mode_t _join_mode;
    bool _is_joined;
    bool _data_comfirm;

    void (*_onReceive)(LoRaWAN_frame_t);
    void (*_onSend)();
    void (*_onJoin)(bool);
    void (*_onError)(char *);

   public:
    bool init(HardwareSerial *serial = &Serial2, int rx = 16, int tx = 17,
              int baudrate = 115200);
    bool configBAND(String band, String channel_mask);
    bool configOTAA(String deveui, String appeui, String appkey);
    bool configABP(String devaddr, String nwkskey, String appskey);

    bool configLinkCheck(LoRaWAN_LinkCheck_t mode);
    bool configADR(bool adr);
    bool configLBT(bool lb);

    bool configDR(uint8_t dr);
    bool configTXP(uint8_t power);

    bool configJoinRX1Delay(uint8_t sec);
    bool configJoinRX2Delay(uint8_t sec);

    bool configRX1Delay(uint8_t sec);
    bool configRX2Delay(uint8_t sec);

    bool configRX2DR(uint8_t dr);
    bool configRX2Freq(uint32_t freq);

    bool configComfirm(bool comfirm);

    bool configLPM(bool enable, uint8_t level = 1);

    bool sleep(uint32_t time_ms);

    bool setMode(LoRaWAN_Dev_Class_t mode);
    bool join(bool enable = true, bool boot_auto_join = false,
              uint8_t retry_interval = 10, uint8_t retry_times = 8);
    size_t send(String data, int port = 1);
    size_t send(const uint8_t *buf, size_t size, int port = 1);

    void parse(String frame);
    void update();

    bool onReceive(void (*callback)(LoRaWAN_frame_t));
    bool onSend(void (*callback)());
    bool onJoin(void (*callback)(bool));
    bool onError(void (*callback)(char *));

    int available();
    std::vector<LoRaWAN_frame_t> read();

    void flush();
};

#endif
