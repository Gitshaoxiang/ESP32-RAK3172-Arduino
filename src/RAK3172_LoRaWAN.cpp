
#include "RAK3172_LoRaWAN.h"

// +EVT:RXP2P:0:12:1231233412453215342523

bool RAK3172_LoRaWAN::init(HardwareSerial *serial, int rx, int tx,
                           int baudrate) {
    RAK3172::init(serial, rx, tx, baudrate);
    delay(100);
    return (sendCommand("AT+NWM=1") && sendCommand("AT"));
}

bool RAK3172_LoRaWAN::configBAND(String band, String channel_mask) {
    return (sendCommand("AT+BAND=" + band) &&
            sendCommand("AT+MASK=" + channel_mask));
}

bool RAK3172_LoRaWAN::configOTAA(String deveui, String appeui, String appkey) {
    _join_mode = OTAA;
    return (sendCommand("AT+NJM=1") && sendCommand("AT+DEVEUI=" + deveui) &&
            sendCommand("AT+APPEUI=" + appeui) &&
            sendCommand("AT+APPKEY=" + appkey));
}

bool RAK3172_LoRaWAN::configABP(String devaddr, String nwkskey,
                                String appskey) {
    _join_mode = ABP;
    return (sendCommand("AT+NJM=0") && sendCommand("AT+DEVADDR=" + devaddr) &&
            sendCommand("AT+NWKSKEY=" + nwkskey) &&
            sendCommand("AT+APPSKEY=" + appskey));
}

bool RAK3172_LoRaWAN::setMode(LoRaWAN_Dev_Class_t mode) {
    if (mode == CLASS_A) {
        return sendCommand("AT+CLASS=A");
    }
    if (mode == CLASS_B) {
        return sendCommand("AT+CLASS=B");
    }
    if (mode == CLASS_C) {
        return sendCommand("AT+CLASS=C");
    }
    return false;
}

bool RAK3172_LoRaWAN::join() {
    if (_join_mode == OTAA) {
        return sendCommand("AT+JOIN=1:0:10:8");
    } else {
        return false;
    }
}

size_t RAK3172_LoRaWAN::send(String data, int port = 1) {
    String hexEncoded = encodeMsg(data);
    if (sendCommand("AT+SEND=" + String(port) + ":" + hexEncoded)) {
        return data.length();
    };
    return 0;
}

size_t RAK3172_LoRaWAN::send(const uint8_t *buf, size_t size, int port = 1) {
    String hexEncoded = bytes2hex(buf, size);
    if (sendCommand("AT+SEND=" + String(port) + ":" + hexEncoded)) {
        return size;
    };
    return 0;
}
