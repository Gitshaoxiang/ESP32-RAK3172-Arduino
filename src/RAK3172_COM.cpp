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
    if ((hexEncoded.length() & 1) == 0) {
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