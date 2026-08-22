/**
 * ui_handler_config.cpp
 * 設定内容一覧画面（CONFIG）の描画処理
 */

#include "ui_handler.h"                         // M5Unified.hはこの中でインクルード済

#include <WiFi.h>                               // WiFi.localIP() と WiFi.macAddress() を使用するため

#include "storage_handler.h"                    // 設定値取得のため

// ============================================================
// 設定内容一覧画面（CONFIG）を描画する
// ============================================================
void drawConfigView() {

    // 画面共通の初期化
    initScreenDrawing();

    // ------------------------------------------------------
    // タイトル
    // ------------------------------------------------------
    drawTitle("CONFIG");

    // ------------------------------------------------------
    // 外枠
    // ------------------------------------------------------
    drawOuterFrameWithTitle();

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
    // sprintf不使用（vfprintf系の実装がリンクされフラッシュ容量を圧迫するため、Stringクラスの機能で代替）
    // 未登録（LOCATION_UNSET）の場合は座標をそのまま表示せず「Not set」と表示する
    String locStr;
    if (config.lat == LOCATION_UNSET || config.lng == LOCATION_UNSET) {
        locStr = "Not set";
    } else {
        locStr = String(config.lat, 3) + " , " + String(config.lng, 3);
    }
    M5.Lcd.setCursor(80, 41);
    M5.Lcd.print(locStr);

    // API（マスク済みキー・残数）
    M5.Lcd.setCursor(12, 60);
    M5.Lcd.print("API");
    M5.Lcd.setCursor(80, 60);
    M5.Lcd.print(maskSecret(apiKey));

    // sprintf不使用（vfprintf系の実装がリンクされフラッシュ容量を圧迫するため、Stringクラスの機能で代替）
    String remainStr = "(" + String(remainingRequests) + " left)";
    M5.Lcd.setCursor(80, 76);
    M5.Lcd.print(remainStr);

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
    // ボタンラベル・区切り線
    // ------------------------------------------------------
    drawButtonLabels("BACK", nullptr, nullptr);
}