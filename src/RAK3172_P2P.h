/*
 PubSubClient.h - A simple client for MQTT.
  Nick O'Leary
  http://knolleary.net
*/

#ifndef _RAK3172_P2P_H
#define _RAK3172_P2P_H

#include <Arduino.h>
#include "Stream.h"
#include <vector>
#include "RAK3172_COM.h"

typedef struct {
    int rssi;
    int snr;
    char payload[500];
} P2P_frame_t;

typedef enum { P2P_TX_MODE = 0, P2P_RX_MODE } P2P_mode_t;

class RAK3172_P2P {
   private:
    HardwareSerial *_serial;
    int _tx_pin;
    int _rx_pin;
    std::vector<P2P_frame_t> _frames;
    P2P_mode_t _mode;

   public:
    bool init(HardwareSerial *serial = &Serial2, int rx = 16, int tx = 17,
              int baudrate = 115200);
    void update();
    bool config(long freq, int sf, int bw, int cr, int prlen, int pwr);
    bool setMode(P2P_mode_t mode, time_t timeout = 65534);
    bool sendCommand(String cmd);
    size_t print(const char *str);
    size_t write(const uint8_t *buf, size_t size);
    int available();
    std::vector<P2P_frame_t> read();
    void parse(String frame);
    size_t debug(String cmd);
    void flush();
};

#endif
