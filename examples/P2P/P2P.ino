#include <M5Unified.h>
#include "M5GFX.h"
#include "lora_config.h"
#include "sf7.h"
#include "sf12.h"
#include "RAK3172.h"

RAK3172_P2P lora;

M5Canvas canvas(&M5.Display);

int msgCount = 0;

int cr       = 0;
int sf       = 12;
int bw       = 125;
int tx_power = 20;

String sent_count_str = "...";
String rssi_str       = "...";
String snr_str        = "...";
String incoming_str   = "...";

#define LORA_CONFIG_PRLEN 8
#define LORA_CONFIG_PWR   22

void updateConfig(String config) {
    M5.Display.setTextDatum(top_center);
    M5.Display.drawString(config, 160, 12);
}

void updateStatus(String product, String snr, String rssi, String data, String count) {
    canvas.clear();
    canvas.drawString("PRODUCT: " + product, 25, 10);
    canvas.drawString("RX SNR: " + snr, 25, 35);
    canvas.drawString("RX RSSI: " + rssi, 25, 65);
    canvas.drawString("RX DATA: " + data, 25, 95);
    canvas.drawString("TX COUNT: " + count, 25, 125);
    canvas.pushSprite(0, 38);
}

void updateUI() {
    sent_count_str = "...";
    rssi_str       = "...";
    snr_str        = "...";
    incoming_str   = "...";
    msgCount       = 1;

    if (sf == 12) {
        M5.Display.pushImage(0, 0, 320, 240, image_data_sf12);
    } else {
        M5.Display.pushImage(0, 0, 320, 240, image_data_sf7);
    }
    updateConfig(String(LORA_FREQ_STR) + " SF: " + String(sf) + " BW: " + String(bw) + "K " + " CR: 4/" +
                 String(cr + 5));
    updateStatus(PRODUCT_NAME, snr_str, rssi_str, incoming_str, sent_count_str);
}

void LoRaLoopTask(void* arg) {
    while (1) {
        lora.update();
        vTaskDelay(5);
    }
}

void beep() {
    M5.Speaker.tone(2000, 100);
    while (M5.Speaker.isPlaying()) {
        M5.update();
    }
}

void setup() {
    M5.begin();  // Init M5Core.  初始化 M5Core
    M5.Display.begin();
    M5.Display.setTextColor(WHITE);
    M5.Display.setFont(&fonts::FreeSansBold9pt7b);
    M5.Display.setTextSize(1);

    canvas.createSprite(320, 140);
    canvas.setFont(&fonts::FreeSansBold9pt7b);
    M5.Speaker.begin();
    M5.Display.print("init...");

    M5.Power.setExtOutput(true);

    // while (!lora.init(&Serial2, 16, 17, 115200)) {
    // while (!lora.init(&Serial2, 13, 14, 115200)) {
    // while (!lora.init(&Serial2, 33, 32, 115200)) {
    // while (!lora.init(&Serial2, 32, 26, 115200)) {
    // while (!lora.init(&Serial2, 33, 32, 115200)) {
    // while (!lora.init(&Serial2, 18, 17, 115200)) {
    while (!lora.init(&Serial2, 16, 17, 115200)) {
        // while (!lora.init(&Serial2, 19, 18, 115200)) {
        // while (!lora.init(&Serial2, 1, 2, 115200)) {
        // while (!lora.init(&Serial2, 15, 13, 115200)) {
        delay(1000);
    }

    if (lora.config(LORA_FREQ, sf, bw, cr, LORA_CONFIG_PRLEN, LORA_CONFIG_PWR)) {
        updateUI();
        beep();
    }

    lora.setMode(P2P_TX_RX_MODE);
    xTaskCreate(LoRaLoopTask, "LoRaLoopTask", 1024 * 10, NULL, 2, NULL);
}

void sleep_mode_test() {
    M5.Display.pushImage(0, 0, 320, 240, image_data_sf7);

    if (lora.setMode(P2P_RX_MODE, 0) && lora.configLPM(false) && lora.configLPMLevel(2)) {
        lora.sleep(0);
        updateConfig("Enter Sleep Mode OK");
    } else {
        updateConfig("Sleep Mode Fail");
        return;
    }
    while (1) {
        M5.update();
        if (M5.BtnB.wasReleased()) {
            break;
        }
    }
    M5.Display.pushImage(0, 0, 320, 240, image_data_sf7);

    while (!lora.sendCommand("AT")) {
        updateConfig("Exit Sleep Mode Fail");
        delay(100);
    }
    updateConfig("Exit Sleep Mode OK");
}

void loop() {
    M5.update();
    if (M5.BtnB.wasReleased()) {
        // set to sleep mode
        // sleep_mode_test();
        sf = 7;
        bw = 500;
        lora.setMode(P2P_RX_MODE, 0);
        if (lora.config(LORA_FREQ, sf, bw, cr, LORA_CONFIG_PRLEN, LORA_CONFIG_PWR)) {
            beep();
            updateUI();
        }
        lora.setMode(P2P_TX_RX_MODE);
    }
    if (M5.BtnC.wasReleased()) {
        sf = 12;
        bw = 125;
        lora.setMode(P2P_RX_MODE, 0);
        if (lora.config(LORA_FREQ, sf, bw, cr, LORA_CONFIG_PRLEN, LORA_CONFIG_PWR)) {
            beep();
            updateUI();
        }
        lora.setMode(P2P_TX_RX_MODE);
    }

    if (M5.BtnA.wasReleased()) {
        String message = "Hello World: " + String(msgCount);
        if (lora.print(message.c_str())) {
            beep();
            Serial.println("Sending " + message);
            sent_count_str = String(msgCount);
            updateStatus(PRODUCT_NAME, snr_str, rssi_str, incoming_str, sent_count_str);
            msgCount++;
        }
    }
    if (lora.available()) {
        std::vector<P2P_frame_t> frames = lora.read();
        for (int i = 0; i < frames.size(); i++) {
            beep();
            Serial.print("RSSI: ");
            Serial.print(frames[i].rssi);
            Serial.print(" SNR: ");
            Serial.print(frames[i].snr);
            Serial.print(" LEN: ");
            Serial.print(frames[i].len);
            Serial.print(" Payload: ");
            Serial.println(frames[i].payload);

            incoming_str   = String(frames[i].payload);
            sent_count_str = String(msgCount);
            snr_str        = String(frames[i].snr);
            rssi_str       = String(frames[i].rssi);

            updateStatus(PRODUCT_NAME, snr_str, rssi_str, incoming_str, sent_count_str);
        }
        lora.flush();
    }
}
