#ifndef _RAK3172_P2P_H
#define _RAK3172_P2P_H

#include <Arduino.h>
#include "Stream.h"
#include <vector>
#include "RAK3172_COM.h"

typedef struct {
    int rssi;
    int snr;
    int len;
    char payload[500];
} P2P_frame_t;

typedef enum { P2P_TX_MODE = 0, P2P_RX_MODE } P2P_mode_t;

class RAK3172_P2P : public RAK3172 {
   private:
    std::vector<P2P_frame_t> _frames;
    P2P_mode_t _mode;

   public:
    bool init(HardwareSerial *serial = &Serial2, int rx = 16, int tx = 17,
              int baudrate = 115200);
    void update();
    bool config(long freq, int sf, int bw, int cr, int prlen, int pwr);
    bool setMode(P2P_mode_t mode, time_t timeout = 65534);
    size_t print(const char *str);
    size_t write(const uint8_t *buf, size_t size);
    int available();
    std::vector<P2P_frame_t> read();
    void parse(String frame);
    void flush();
};

#endif
