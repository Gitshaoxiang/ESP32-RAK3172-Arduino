#include <M5Unified.h>
#include "M5GFX.h"

#include "background_p2p.h"

M5Canvas canvas_status(&M5.Display);
M5Canvas canvas_payload(&M5.Display);

#define DARK_BG_COLOR 0x2124

#ifndef NATIVE_PLATFORM

#include "RAK3172.h"

#define LORA_CONFIG_FREQ  920000000
#define LORA_CONFIG_SF    7
#define LORA_CONFIG_BW    125
#define LORA_CONFIG_CR    1
#define LORA_CONFIG_PRLEN 8
#define LORA_CONFIG_PWR   20

// Bandwidth = {125, 250, 500}
// SF = {5, 6, 7, 8, 9, 10, 11, 12}
// CR = {4/5=0, 4/6=1, 4/7=2, 4/8=3}

RAK3172_P2P lora;

void LoRaLoopTask(void* arg) {
    while (1) {
        lora.update();
        vTaskDelay(5);
    }
}

void updatePayload(const char* payload) {
    canvas_payload.fillScreen(DARK_BG_COLOR);
    canvas_payload.setCursor(0, 0);
    canvas_payload.print(payload);
    canvas_payload.pushSprite(142, 86);
}

void updateConfig(P2P_mode_t mode) {
    M5.Display.pushImage(0, 0, 320, 240, image_data_background);
    if (mode == P2P_RX_MODE) {
        M5.Display.drawString("LoRa RX: 920M SF7 125K CR 4/6", 160, 20);
        updatePayload("Waiting For Data...");

    } else {
        M5.Display.drawString("LoRa TX: 920M SF7 125K CR 4/6", 160, 20);
        updatePayload("Click BtnB To Send Data");
    }
    canvas_status.fillScreen(DARK_BG_COLOR);
    canvas_status.pushSprite(15, 86);
}

void updateTXStatus(const char* payload, int len, bool flag) {
    updatePayload(payload);
    canvas_status.fillScreen(DARK_BG_COLOR);
    canvas_status.setCursor(10, 25);
    if (flag) {
        canvas_status.setTextColor(GREEN);
        canvas_status.print("发送成功");
    } else {
        canvas_status.setTextColor(RED);
        canvas_status.print("发送失败");
    }

    canvas_status.setCursor(10, 0);
    canvas_status.print("LEN: ");
    canvas_status.print(len);

    canvas_status.pushSprite(15, 86);
}

void updateRXStatus(int rssi, int snr, int len) {
    canvas_status.fillScreen(DARK_BG_COLOR);
    canvas_status.setTextColor(WHITE);
    canvas_status.setCursor(10, 0);
    canvas_status.print("RSSI: ");
    canvas_status.print(rssi);
    canvas_status.setCursor(10, 25);
    canvas_status.print("LEN: ");
    canvas_status.print(len);
    canvas_status.setCursor(10, 50);
    canvas_status.print("SNR: ");
    canvas_status.print(snr);
    canvas_status.setCursor(10, 75);

    if (rssi > -10 && snr > 4) {
        canvas_status.setTextColor(GREEN);
        canvas_status.print("测试通过");
    } else {
        canvas_status.setTextColor(RED);
        canvas_status.print("信号不通过");
    }
    canvas_status.pushSprite(15, 86);
}

#endif

void setup() {
    M5.begin();  // Init M5Core.  初始化 M5Core
    M5.Display.begin();
    M5.Display.pushImage(0, 0, 320, 240, image_data_background);

    // M5.Display.setTextColor(BLACK);
    M5.Display.setTextColor(DARK_BG_COLOR);
    M5.Display.setTextFont(&fonts::FreeSansBold9pt7b);
    M5.Display.setTextDatum(top_center);

    canvas_status.createSprite(100, 90);
    canvas_payload.createSprite(160, 90);

    canvas_status.fillScreen(DARK_BG_COLOR);
    canvas_payload.fillScreen(DARK_BG_COLOR);

    canvas_status.setTextColor(WHITE);
    canvas_payload.setTextColor(WHITE);
    canvas_payload.setTextPadding(2);
    canvas_status.setTextFont(&fonts::efontCN_14_b);
    canvas_payload.setTextFont(&fonts::efontCN_14_b);
    canvas_payload.setTextWrap(true);

#ifndef NATIVE_PLATFORM
    Serial.setTimeout(200);

    // if (lora.init(&Serial2, 22, 21, 115200)) {
    while (!lora.init(&Serial2, 16, 17, 115200)) {
        delay(1000);
    }
    Serial.println("Device Init OK");

    while (!lora.config(LORA_CONFIG_FREQ, LORA_CONFIG_SF, LORA_CONFIG_BW,
                        LORA_CONFIG_CR, LORA_CONFIG_PRLEN, LORA_CONFIG_PWR)) {
        delay(1000);
    }
    Serial.println("Config Init OK");
    while (!lora.setMode(P2P_TX_MODE)) {
        delay(1000);
    }
    updateConfig(P2P_TX_MODE);
    Serial.println("TX Mode Init OK");

    xTaskCreate(LoRaLoopTask, "LoRaLoopTask", 1024 * 10, NULL, 5, NULL);
#endif
}

void loop() {
    M5.update();

    // if (Serial.available()) {
    //     Serial2.write(Serial.read());
    // }

    // if (Serial2.available()) {
    //     Serial.write(Serial2.read());
    // }
#ifndef NATIVE_PLATFORM
    if (M5.BtnA.wasReleased()) {
        if (lora.setMode(P2P_TX_MODE)) {
            Serial.println("Init TX OK");
            updateConfig(P2P_TX_MODE);
        } else {
            Serial.println("Init TX Fail");
        }
    }
    if (M5.BtnC.wasReleased()) {
        if (lora.setMode(P2P_RX_MODE)) {
            Serial.println("Init RX OK");
            updateConfig(P2P_RX_MODE);
        } else {
            Serial.println("Init RX Fail");
        }
    }
    if (M5.BtnB.wasReleased()) {
        // uint8_t data[] = {0x01, 0x02, 0x03, 0x04};
        // if (lora.write(data, sizeof(data))) {
        //     Serial.println("Write OK");
        // }
        // delay(500);
        String data = "Hello LoRa: " + String(millis());
        if (lora.print(data.c_str())) {
            updateTXStatus(data.c_str(), data.length(), true);
        } else {
            updateTXStatus(data.c_str(), data.length(), false);
        }
    }
    if (lora.available()) {
        std::vector<P2P_frame_t> frames = lora.read();
        for (int i = 0; i < frames.size(); i++) {
            Serial.print("RSSI: ");
            Serial.print(frames[i].rssi);
            Serial.print(" SNR: ");
            Serial.print(frames[i].snr);
            Serial.print(" LEN: ");
            Serial.print(frames[i].len);
            Serial.print(" Payload: ");
            Serial.println(frames[i].payload);

            updateRXStatus(frames[i].rssi, frames[i].snr, frames[i].len);
            updatePayload(frames[i].payload);
        }
        lora.flush();
    }
    if (Serial.available()) {  // If the serial port reads data.
        String ch =
            Serial.readString();  // Copy the data read from the serial port
        lora.sendCommand(ch);
    }
#endif
}
