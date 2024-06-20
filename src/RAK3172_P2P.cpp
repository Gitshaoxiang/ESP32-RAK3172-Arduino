
#include "RAK3172_P2P.h"

// +EVT:RXP2P:0:12:1231233412453215342523

void RAK3172_P2P::parse(String frame) {
    P2P_frame_t res;
    int index = frame.indexOf("+EVT:RXP2P:");
    if (index != -1) {
        String tmp     = frame.substring(index + 11);
        int rssi       = tmp.substring(0, tmp.indexOf(":")).toInt();
        tmp            = tmp.substring(tmp.indexOf(":") + 1);
        int snr        = tmp.substring(0, tmp.indexOf(":")).toInt();
        tmp            = tmp.substring(tmp.indexOf(":") + 1);
        String payload = decodeMsg(tmp);
        res.rssi       = rssi;
        res.snr        = snr;
        res.len        = payload.length();
        payload.toCharArray(res.payload, payload.length() + 1);
        _frames.push_back(res);
    }
}

void RAK3172_P2P::update() {
    if (xSemaphoreTake(_serial_mutex, portMAX_DELAY) == pdTRUE) {
        String res = _serial->readStringUntil('\n');
        xSemaphoreGive(_serial_mutex);
        Serial.print(res);
        if (res.indexOf("+EVT:RXP2P") != -1) {
            if (res.indexOf("ERROR") != -1) {
            } else {
                res.remove(res.length() - 1);
                parse(res);
            }
        }
        if (res.indexOf("+EVT:TXP2P DONE") != -1) {
        }
    }
}

bool RAK3172_P2P::init(HardwareSerial *serial, int rx, int tx, int baudrate) {
    RAK3172::init(serial, rx, tx, baudrate);
    restart();
    delay(100);
    return (sendCommand("AT") && sendCommand("AT+NWM=0"));
}

bool RAK3172_P2P::restart() {
    sendCommand("ATZ");
    return true;
}

bool RAK3172_P2P::config(long freq, int sf, int bw, int cr, int prlen,
                         int pwr) {
    // AT+P2P=868000000:12:125:3:200:14
    return sendCommand("AT+P2P=" + String(freq) + ":" + String(sf) + ":" +
                       String(bw) + ":" + String(cr) + ":" + String(prlen) +
                       ":" + String(pwr));
}

bool RAK3172_P2P::setMode(P2P_mode_t mode, time_t timeout) {
    bool status = false;
    if (mode == P2P_TX_MODE) {
        status = sendCommand("AT+PRECV=0");
    }
    if (mode == P2P_RX_MODE) {
        status = sendCommand("AT+PRECV=" + String(timeout));
    }

    if (mode == P2P_TX_RX_MODE) {
        status = sendCommand("AT+PRECV=65533");
    }

    if (status) {
        _mode = mode;
    }
    return status;
}

int RAK3172_P2P::available() {
    return _frames.size();
}

// 2~500 位长度，必须是偶数位和字符 0-9, a-f, A-F组成，代表1~256个十六进制数。
size_t RAK3172_P2P::write(const uint8_t *buf, size_t size) {
    String hexEncoded = bytes2hex(buf, size);
    if (sendCommand("AT+PSEND=" + hexEncoded)) {
        return size;
    };
    return 0;
}

size_t RAK3172_P2P::print(const char *str) {
    String hexEncoded = encodeMsg(String(str));
    if (sendCommand("AT+PSEND=" + hexEncoded)) {
        return strlen(str);
    };
    return 0;
}

std::vector<P2P_frame_t> RAK3172_P2P::read() {
    return _frames;
}

void RAK3172_P2P::flush() {
    _serial->flush();
    _frames.clear();
}
