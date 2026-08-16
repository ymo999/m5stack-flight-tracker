/**
 * ui_handler_config.cpp
 * 設定内容一覧画面（CONFIG）の描画処理
 */

#include "ui_handler.h"                         // M5Unified.hはこの中でインクルード済

#include <WiFi.h>

#include "storage_handler.h"

// ============================================================
// 設定内容一覧画面（CONFIG）を描画する
// ============================================================
void drawConfigView() {

    // 画面共通の初期化
    initScreenDrawing();

    // ------------------------------------------------------
    // タイトル（中央揃え、size(2)のまま）
    // ------------------------------------------------------
    M5.Lcd.setTextDatum(top_center);
    M5.Lcd.drawString("CONFIG", 160, 8);
    M5.Lcd.setTextDatum(top_left);

    // ------------------------------------------------------
    // 外枠
    // ------------------------------------------------------
    M5.Lcd.drawRect(5, 34, 310, 200, TFT_WHITE);

    // ------------------------------------------------------
    // データ読み込み
    // ------------------------------------------------------
    ConfigData config;
    loadConfig(config);

    String apiKey;
    loadApiKey(apiKey);

    int remainingRequests = 0;
    loadRemainingRequests(remainingRequests);

    String ssid;
    loadWifiSsid(ssid);

    // ------------------------------------------------------
    // 項目一覧（ラベル・値ともにsize(1)に変更）
    // ------------------------------------------------------
    M5.Lcd.setTextSize(1);

    // LOCATION（緯度・経度）
    M5.Lcd.setCursor(12, 41);
    M5.Lcd.print("LOCATION");
    char locBuf[24];
    sprintf(locBuf, "%.3f , %.3f", config.lat, config.lng);
    M5.Lcd.setCursor(80, 41);
    M5.Lcd.print(locBuf);

    // API（マスク済みキー・残数）
    M5.Lcd.setCursor(12, 60);
    M5.Lcd.print("API");
    M5.Lcd.setCursor(80, 60);
    M5.Lcd.print(maskSecret(apiKey));

    char remainBuf[16];
    sprintf(remainBuf, "(%d left)", remainingRequests);
    M5.Lcd.setCursor(80, 76);
    M5.Lcd.print(remainBuf);

    // Wi-Fi（SSID・IP・MAC）
    M5.Lcd.setCursor(12, 95);
    M5.Lcd.print("Wi-Fi");

    M5.Lcd.setCursor(80, 95);
    M5.Lcd.print(ssid);

    M5.Lcd.setCursor(80, 111);
    M5.Lcd.print(WiFi.localIP().toString());

    M5.Lcd.setCursor(80, 127);
    M5.Lcd.print(WiFi.macAddress());

    // SCAN RANGE
    M5.Lcd.setCursor(12, 146);
    M5.Lcd.print("SCAN RANGE");
    M5.Lcd.setCursor(80, 146);
    M5.Lcd.print(config.scanRange);

    // ------------------------------------------------------
    // ボタンラベル・区切り線（size(2)のまま）
    // ------------------------------------------------------
    M5.Lcd.setTextSize(2);
    drawButtonLabels("BACK", nullptr, nullptr);
}