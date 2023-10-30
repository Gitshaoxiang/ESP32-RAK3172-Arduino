
#include "RAK3172_P2P.h"

SemaphoreHandle_t LoRa_P2P_Mutex;

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
        payload.toCharArray(res.payload, payload.length() + 1);
        _frames.push_back(res);
    }
}

void RAK3172_P2P::update() {
    String res = "";
    if (xSemaphoreTake(LoRa_P2P_Mutex, portMAX_DELAY) == pdTRUE) {
        String res = _serial->readStringUntil('\n');
        xSemaphoreGive(LoRa_P2P_Mutex);
        if (res.indexOf("+EVT:RXP2P") != -1) {
            Serial.print(res);
            parse(res);
        }
    }
}

size_t RAK3172_P2P::debug(String cmd) {
    size_t size = 0;
    if (xSemaphoreTake(LoRa_P2P_Mutex, portMAX_DELAY) == pdTRUE) {
        size = _serial->print(cmd);
        xSemaphoreGive(LoRa_P2P_Mutex);
    }
    return size;
}

bool RAK3172_P2P::sendCommand(String cmd) {
    if (xSemaphoreTake(LoRa_P2P_Mutex, portMAX_DELAY) == pdTRUE) {
        _serial->println(cmd);
        Serial.println("SEND CMD: ");
        Serial.println(cmd);
        String res = _serial->readStringUntil('\n');
        Serial.println("RESPONSE: ");
        Serial.println(res);
        xSemaphoreGive(LoRa_P2P_Mutex);
        if (res.indexOf("OK") != -1) {
            return true;
        }
    }
    return false;
}

bool RAK3172_P2P::init(HardwareSerial *serial, int rx, int tx, int baudrate) {
    _serial = serial;
    _tx_pin = tx;
    _rx_pin = rx;
    _serial->begin(baudrate, SERIAL_8N1, rx, tx);
    _serial->setTimeout(50);
    LoRa_P2P_Mutex = xSemaphoreCreateMutex();
    xSemaphoreGive(LoRa_P2P_Mutex);

    return (sendCommand("AT") && sendCommand("AT+NWM=0"));
}

bool RAK3172_P2P::config(long freq, int sf, int bw, int cr, int prlen,
                         int pwr) {
    return (sendCommand("AT+PFREQ=" + String(freq)) &&
            sendCommand("AT+PSF=" + String(sf)) &&
            sendCommand("AT+PBW=" + String(bw)) &&
            sendCommand("AT+PCR=" + String(cr)) &&
            sendCommand("AT+PPL=" + String(prlen)) &&
            sendCommand("AT+PTP=" + String(pwr)));
}

bool RAK3172_P2P::setMode(P2P_mode_t mode, time_t timeout) {
    bool status = false;
    if (mode == P2P_TX_MODE) {
        status = sendCommand("AT+PRECV=0");
    }
    if (mode == P2P_RX_MODE) {
        status = sendCommand("AT+PRECV=" + String(timeout));
    }
    if (status) {
        _mode = mode;
    }
    return false;
}

int RAK3172_P2P::available() {
    return _frames.size();
}

// 2~500 位长度，必须是偶数位和字符 0-9, a-f, A-F组成，代表1~256个十六进制数。
size_t RAK3172_P2P::write(const uint8_t *buf, size_t size) {
    if (xSemaphoreTake(LoRa_P2P_Mutex, portMAX_DELAY) == pdTRUE) {
        _serial->print("AT+PSEND=");
        String hexEncoded = bytes2hex(buf, size);
        _serial->print(hexEncoded);
        _serial->println();
        String res = _serial->readStringUntil('\n');
        Serial.println("RESPONSE: ");
        Serial.println(res);

        xSemaphoreGive(LoRa_P2P_Mutex);
        if (res.indexOf("+EVT:TXP2P DONE") != -1) {
            Serial.print(res);
            return size;
        }
    }
    return 0;
}

size_t RAK3172_P2P::print(const char *str) {
    String res = "";
    if (xSemaphoreTake(LoRa_P2P_Mutex, portMAX_DELAY) == pdTRUE) {
        _serial->print("AT+PSEND=");
        String hexEncoded = encodeMsg(String(str));
        _serial->print(hexEncoded);
        _serial->println();
        String res = _serial->readStringUntil('\n');
        Serial.println("RESPONSE: ");
        Serial.println(res);
        xSemaphoreGive(LoRa_P2P_Mutex);
        if (res.indexOf("+EVT:TXP2P DONE") != -1) {
            Serial.print(res);
            return strlen(str);
        }
    }
    return 0;
}

std::vector<P2P_frame_t> RAK3172_P2P::read() {
    return _frames;
}

void RAK3172_P2P::flush() {
    _serial->flush();
    _frames.clear();
}
