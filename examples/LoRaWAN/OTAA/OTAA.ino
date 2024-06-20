
#include <M5Unified.h>
#include "M5GFX.h"

#include "background_lorawan.h"

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

void updatePayload(const char* payload) {
    canvas_payload.fillScreen(DARK_BG_COLOR);
    canvas_payload.setCursor(0, 0);
    canvas_payload.print(payload);
    canvas_payload.pushSprite(142, 86);
}

void updateTitle(const char* str) {
    M5.Display.pushImage(0, 0, 320, 240, image_data_background);
    M5.Display.drawString(str, 160, 20);
    canvas_status.fillScreen(DARK_BG_COLOR);
    canvas_status.pushSprite(15, 86);
    canvas_payload.pushSprite(142, 86);
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

void updateRXStatus(int rssi, int snr, int port, int len) {
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
    canvas_status.print("PORT: ");
    canvas_status.print(port);
    canvas_status.pushSprite(15, 86);
}

void joinCallback(bool status) {
    if (status) {
        String title = "Joined! EUI:" + String(DEVEUI);
        updateTitle(title.c_str());
    }
}

void receiveCallback(LoRaWAN_frame_t data) {
}

void sendCallback() {
}

void errorCallback(char* error) {
}

void LoRaWANLoopTask(void* arg) {
    while (1) {
        lorawan.update();
        vTaskDelay(5);
    }
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

    // while (!lorawan.setMode(CLASS_C)) {
    while (!lorawan.setMode(CLASS_A)) {
        delay(1000);
    }

    while (!lorawan.configDR(4)) {
        delay(1000);
    }

    while (!lorawan.configLinkCheck(ALLWAYS_LINKCHECK)) {
        delay(1000);
    }

    lorawan.onReceive(receiveCallback);
    lorawan.onSend(sendCallback);
    lorawan.onJoin(joinCallback);
    lorawan.onError(errorCallback);

    Serial.println("Config Init OK");

    updateTitle("Click BtnA Start Join...");
    updatePayload("Click BtnA Start Join...");

    xTaskCreate(LoRaWANLoopTask, "LoRaWANLoopTask", 1024 * 10, NULL, 5, NULL);
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
        if (lorawan.join(true, false, 10, 8)) {
            Serial.println("Start Join...");
        } else {
            Serial.println("Join Fail");
        }
    }
    if (M5.BtnC.wasReleased()) {
        if (lorawan.sleep(3000)) {
            Serial.println("sleep...");
        }

        //  else {
        //     Serial.println("sleep Fail");
        // }
    }
    if (M5.BtnB.wasReleased()) {
        String data = "UPlink LoRaWAN Frame: " + String(millis());
        if (lorawan.send(data)) {
            updateTXStatus(data.c_str(), data.length(), true);
        } else {
            updateTXStatus(data.c_str(), data.length(), false);
        }
    }
    if (lorawan.available()) {
        std::vector<LoRaWAN_frame_t> frames = lorawan.read();
        for (int i = 0; i < frames.size(); i++) {
            Serial.print("RSSI: ");
            Serial.println(frames[i].rssi);
            Serial.print("SNR: ");
            Serial.println(frames[i].snr);
            Serial.print("LEN: ");
            Serial.println(frames[i].len);
            Serial.print("PORT: ");
            Serial.println(frames[i].port);
            Serial.print("UNITCAST: ");
            Serial.println(frames[i].unicast);
            Serial.print("Payload: ");
            Serial.println(frames[i].payload);
            updateRXStatus(frames[i].rssi, frames[i].snr, frames[i].port, frames[i].len);
            updatePayload(frames[i].payload);
        }
        lorawan.flush();
    }
    if (Serial.available()) {             // If the serial port reads data.
        String ch = Serial.readString();  // Copy the data read from the serial port
        lorawan.sendCommand(ch);
    }
#endif
}
