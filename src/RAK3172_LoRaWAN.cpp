
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

bool RAK3172_LoRaWAN::configLinkCheck(LoRaWAN_LinkCheck_t mode) {
    if (mode == DIS_LINKCHECK) {
        return sendCommand("AT+LINKCHECK=0");
    }
    if (mode == ONCE_LINKCHECK) {
        return sendCommand("AT+LINKCHECK=1");
    }
    if (mode == ALLWAYS_LINKCHECK) {
        return sendCommand("AT+LINKCHECK=2");
    }
    return false;
}

bool RAK3172_LoRaWAN::configADR(bool adr) {
    if (adr) {
        return sendCommand("AT+ADR=1");
    } else {
        return sendCommand("AT+ADR=0");
    }
}

bool RAK3172_LoRaWAN::configLBT(bool lb) {
    if (lb) {
        return sendCommand("AT+LBT=1");
    } else {
        return sendCommand("AT+LBT=0");
    }
}

bool RAK3172_LoRaWAN::join(bool enable, bool boot_auto_join,
                           uint8_t retry_interval, uint8_t retry_times) {
    if (_join_mode == OTAA) {
        return sendCommand("AT+JOIN=" + String(enable) + ":" +
                           String(boot_auto_join) + ":" +
                           String(retry_interval) + ":" + String(retry_times));
    } else {
        return false;
    }
}

bool RAK3172_LoRaWAN::configJoinRX1Delay(uint8_t sec) {
    if (_join_mode == OTAA) {
        return sendCommand("AT+JN1DL=" + String(sec));
    } else {
        return false;
    }
}

bool RAK3172_LoRaWAN::configJoinRX2Delay(uint8_t sec) {
    if (_join_mode == OTAA) {
        return sendCommand("AT+JN2DL=" + String(sec));
    } else {
        return false;
    }
}

bool RAK3172_LoRaWAN::configRX1Delay(uint8_t sec) {
    return sendCommand("AT+RX1DL=" + String(sec));
}

bool RAK3172_LoRaWAN::configRX2Delay(uint8_t sec) {
    return sendCommand("AT+RX2DL=" + String(sec));
}

bool RAK3172_LoRaWAN::configRX2DR(uint8_t dr) {
    return sendCommand("AT+RX2DR=" + String(dr));
}

bool RAK3172_LoRaWAN::configDR(uint8_t dr) {
    return sendCommand("AT+DR=" + String(dr));
}

bool RAK3172_LoRaWAN::configRX2Freq(uint32_t freq) {
    return sendCommand("AT+RX2FQ=" + String(freq));
}

bool RAK3172_LoRaWAN::configTXP(uint8_t power) {
    return sendCommand("AT+TXP=" + String(power));
}

bool RAK3172_LoRaWAN::configComfirm(bool comfirm) {
    if (comfirm) {
        return sendCommand("AT+CFM=1");
    } else {
        return sendCommand("AT+CFM=0");
    }
}

size_t RAK3172_LoRaWAN::send(String data, int port) {
    String hexEncoded = encodeMsg(data);
    if (sendCommand("AT+SEND=" + String(port) + ":" + hexEncoded)) {
        return data.length();
    };
    return 0;
}

size_t RAK3172_LoRaWAN::send(const uint8_t *buf, size_t size, int port) {
    String hexEncoded = bytes2hex(buf, size);
    if (sendCommand("AT+SEND=" + String(port) + ":" + hexEncoded)) {
        return size;
    };
    return 0;
}

void RAK3172_LoRaWAN::parse(String frame) {
    LoRaWAN_frame_t res;
    // +EVT:RX_1:-38:13:UNICAST:1:12312312
    int index = frame.indexOf("+EVT:RX_");
    if (index != -1) {
        String tmp     = frame.substring(index + 10);
        int rssi       = tmp.substring(0, tmp.indexOf(":")).toInt();
        tmp            = tmp.substring(tmp.indexOf(":") + 1);
        int snr        = tmp.substring(0, tmp.indexOf(":")).toInt();
        tmp            = tmp.substring(tmp.indexOf(":") + 1);
        bool unicast   = tmp.substring(0, tmp.indexOf(":")) == "UNICAST";
        tmp            = tmp.substring(tmp.indexOf(":") + 1);
        int port       = tmp.substring(0, tmp.indexOf(":")).toInt();
        tmp            = tmp.substring(tmp.indexOf(":") + 1);
        String payload = decodeMsg(tmp);
        res.rssi       = rssi;
        res.snr        = snr;
        res.unicast    = unicast;
        res.port       = port;
        res.len        = payload.length();
        payload.toCharArray(res.payload, payload.length() + 1);
        _frames.push_back(res);
    }
}

void RAK3172_LoRaWAN::update() {
    String res = "";
    if (xSemaphoreTake(_serial_mutex, portMAX_DELAY) == pdTRUE) {
        String res = _serial->readStringUntil('\n');
        xSemaphoreGive(_serial_mutex);
        Serial.print(res);
        if (res.indexOf("+EVT:LINKCHECK") != -1) {
        }

        if (res.indexOf("+EVT:JOINED") != -1) {
            if (_onJoin) {
                _onJoin(true);
            }
        }
        if (res.indexOf("+EVT:JOIN_FAILED") != -1) {
            if (_onJoin) {
                _onJoin(false);
            }
        }
        if (res.indexOf("+EVT:TX_DONE") != -1) {
            if (_onSend) {
                // _onSend();
            }
        }
        if (res.indexOf("+EVT:RX_") != -1) {
            res.remove(res.length() - 1);
            parse(res);
        }
    }
}

bool RAK3172_LoRaWAN::onReceive(void (*callback)(LoRaWAN_frame_t)) {
    _onReceive = callback;
    return true;
}

bool RAK3172_LoRaWAN::onSend(void (*callback)()) {
    _onSend = callback;
    return true;
}

bool RAK3172_LoRaWAN::onJoin(void (*callback)(bool)) {
    _onJoin = callback;
    return true;
}

bool RAK3172_LoRaWAN::onError(void (*callback)(char *)) {
    _onError = callback;
    return true;
}

int RAK3172_LoRaWAN::available() {
    return _frames.size();
}

std::vector<LoRaWAN_frame_t> RAK3172_LoRaWAN::read() {
    return _frames;
}

void RAK3172_LoRaWAN::flush() {
    _serial->flush();
    _frames.clear();
}
