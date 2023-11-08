
#include <M5Unified.h>
#include "M5GFX.h"

#include "background_p2p.h"

M5Canvas canvas_status(&M5.Display);
M5Canvas canvas_payload(&M5.Display);

#define DARK_BG_COLOR 0x2124

#ifndef NATIVE_PLATFORM

#define DEVEUI       "70b3d57ed0060207"
#define APPEUI       "1254234654362354"
#define APPKEY       "C4E1D2002615315851302068D897E2B4"
#define CHANNEL_MASK "0001"

#include "RAK3172.h"

RAK3172_LoRaWAN lorawan;

// void LoRaLoopTask(void* arg) {
//     while (1) {
//         lora.update();
//         vTaskDelay(5);
//     }
// }

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

    while (!lorawan.init(&Serial2, 16, 17, 115200)) {
        delay(1000);
    }

    Serial.println("Device Init OK");

    while (!lorawan.configBAND(US915, CHANNEL_MASK)) {
        delay(1000);
    }

    while (!lorawan.configOTAA(DEVEUI, APPEUI, APPKEY)) {
        delay(1000);
    }

    while (!lorawan.setMode(CLASS_C)) {
        delay(1000);
    }
    Serial.println("Config Init OK");
    updateConfig(P2P_TX_MODE);
    Serial.println("TX Mode Init OK");

    if (lorawan.join()) {
        Serial.println("Start Join...");
    }
    // xTaskCreate(LoRaLoopTask, "LoRaLoopTask", 1024 * 10, NULL, 5, NULL);
#endif
}

void loop() {
    M5.update();

    if (Serial.available()) {
        Serial2.write(Serial.read());
    }

    if (Serial2.available()) {
        Serial.write(Serial2.read());
    }
#ifndef NATIVE_PLATFORM

#endif
}
