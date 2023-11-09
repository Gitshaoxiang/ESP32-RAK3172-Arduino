#include "RAK3172_COM.h"

String encodeMsg(String str) {
    char buf[str.length() + 1];
    char tempbuf[((str.length() + 1) * 2)];
    str.toCharArray(buf, str.length() + 1);
    int i = 0;
    for (const char *p = buf; *p; ++p) {
        sprintf((char *)(tempbuf + i), "%02x", *p);
        i += 2;
    }
    return String(tempbuf);
}

String decodeMsg(String hexEncoded) {
    if ((hexEncoded.length() % 2) == 0) {
        char buf[hexEncoded.length() + 1];
        char tempbuf[((hexEncoded.length() + 1))];
        hexEncoded.toCharArray(buf, hexEncoded.length() + 1);
        int i = 0;
        for (int loop = 2; loop < hexEncoded.length() + 1; loop += 2) {
            String tmpstr = hexEncoded.substring(loop - 2, loop);
            sprintf(&tempbuf[i], "%c", strtoul(tmpstr.c_str(), nullptr, 16));
            i++;
        }
        return String(tempbuf);
    } else {
        return hexEncoded;
    }
}

String bytes2hex(const uint8_t *buf, size_t size) {
    String res = "";
    for (int i = 0; i < size; i++) {
        if (buf[i] <= 0x0f) {
            res += "0";
        }
        res += String(buf[i], HEX);
    }
    return res;
}

void hex2bytes(String hexEncoded, uint8_t *buf, size_t size) {
    if ((hexEncoded.length() & 1) == 0) {
        char tempbuf[((hexEncoded.length() + 1))];
        hexEncoded.toCharArray(tempbuf, hexEncoded.length() + 1);
        int i = 0;
        for (int loop = 2; loop < hexEncoded.length() + 1; loop += 2) {
            String tmpstr = hexEncoded.substring(loop - 2, loop);
            buf[i]        = strtoul(tmpstr.c_str(), nullptr, 16);
            i++;
        }
    }
}

long hex2bin(String hex) {
    if ((hex.length() & 1) == 0) {
        char buf[hex.length() + 1];
        hex.toCharArray(buf, hex.length() + 1);
        int i     = 0;
        long byte = 0;
        for (int loop = 2; loop < hex.length() + 1; loop += 2) {
            String tmpstr = hex.substring(loop - 2, loop);
            byte          = byte << 8;
            byte |= strtoul(tmpstr.c_str(), nullptr, 16);
        }
        return byte;
    } else {
        return -1;
    }
}

// +EVT:RXP2P:0:12:1231233412453215342523

size_t RAK3172::debug(String cmd) {
    size_t size = 0;
    if (xSemaphoreTake(_serial_mutex, portMAX_DELAY) == pdTRUE) {
        size = _serial->print(cmd);
        xSemaphoreGive(_serial_mutex);
    }
    return size;
}

bool RAK3172::sendCommand(String cmd) {
    if (xSemaphoreTake(_serial_mutex, portMAX_DELAY) == pdTRUE) {
        _serial->println(cmd);
        Serial.print("SEND CMD: ");
        Serial.println(cmd);
        // String res = _serial->readStringUntil('\n');
        String res = _serial->readString();
        Serial.print("RESPONSE: ");
        Serial.println(res);
        xSemaphoreGive(_serial_mutex);
        if (res.indexOf("OK") != -1) {
            return true;
        }
    }
    return false;
}

bool RAK3172::init(HardwareSerial *serial, int rx, int tx, int baudrate) {
    _serial = serial;
    _tx_pin = tx;
    _rx_pin = rx;
    _serial->setTimeout(100);
    _serial->begin(baudrate, SERIAL_8N1, rx, tx);
    _serial_mutex = xSemaphoreCreateMutex();
    xSemaphoreGive(_serial_mutex);
    return sendCommand("AT");
}
