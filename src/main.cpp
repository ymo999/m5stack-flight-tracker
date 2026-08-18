#include <Arduino.h>
#include <DNSServer.h>
#include <M5Unified.h>
#include <WebServer.h>
#include <WiFi.h>

#include "api_handler.h"                // テストコード用
#include "error_data.h"                 // テストコード用（エラー画面確認用）
#include "flight_data.h"                // テストコード用
#include "input_handler.h"
#include "secrets.h"                    // AIRLABS_API_KEY（テストコード用、Git管理外）
#include "state_machine.h"
#include "storage_handler.h"            // テストコード用
#include "system_status.h"              // 物理ボタンを持たない機種にチャタリング防止措置を適用させるため
#include "ui_handler.h"
#include "wifi_handler.h"

void setup() {

    // シリアル通信の初期化（ボーレートはplatformio.ini/モニタ側の設定と合わせる）※デバッグ用
    Serial.begin(115200);
    // Serial.println("[BOOT] Serial initialized");

    // M5Stack本体の初期化
    // 起動の高速化および省電力化のため使用しない機能を明示的に無効化する
    auto cfg = M5.config();
    cfg.internal_imu = false;                           // 加速度センサー無効化
    cfg.internal_mic = false;                           // マイク無効化
    cfg.internal_spk = false;                           // スピーカー無効化（ブザー通知は現状使用しない想定）

    // Wi-Fi関連処理よりも先に記述すること
    // （CoreS3でウォッチドッグタイマーによるリセットがかかりフリーズする既知の現象を回避）
    M5.begin(cfg);
    M5.Power.begin();
    // Serial.println("[BOOT] M5.begin() done");

    // ボタンデバウンスを30msに設定（チャタリング防止）
    // 物理ボタンを持たない機種のみ対象（デフォルト値10msだとタッチのノイズを拾いやすいため。
    // Basic等、物理ボタンのある機種はデフォルトのまま維持する）
    if (isVirtualButtonBoard())
    {
        M5.BtnA.setDebounceThresh(30);
        M5.BtnB.setDebounceThresh(30);
        M5.BtnC.setDebounceThresh(30);
    }
    
    // ===== 調査用一時コード（ここから） =====
    // USB CDC接続の確立を待つための待機
    // モニターが安定接続する前のログ欠落を防ぐための調査用処置
    // 確認後は削除すること

    // ミリ秒単位で時間を測定
    uint32_t startTime = millis();
    
    // 「シリアル未準備」かつ「開始から3秒以内」の間ループする
    while (!Serial && (millis() - startTime < 3000)) {
        delay(100);                         // 接続を待つ
    }
    // ===== 調査用一時コード（ここまで） =====
    
    initStorage();
    // Serial.println("[BOOT] initStorage() done");
    initWiFi();
    // Serial.println("[BOOT] initWiFi() done");
    initStateMachine();
    // Serial.println("[BOOT] initStateMachine() done");
    // Serial.println("[BOOT] setup() complete");
    
    if (isWiFiConnected()) {
        
        // Serial.print("[BOOT] Wi-Fi connected(");
        // Serial.print(WiFi.localIP().toString());
        // Serial.println(")");

    }

    // ★★★一時テストコード↓ここから
    // ※起動時分岐ロジックが未実装のため、暫定的にここでAPIキー・基準地点を仮登録し、
    // 　機体情報を取得してMODE_FLIGHT_VIEWへ遷移させる。
    // 　本来はAPIキー・基準地点の登録状況に応じた分岐（QR_VIEW等）が必要。8.2-C実装時に置き換え予定）

    saveApiKey(AIRLABS_API_KEY);
    // saveApiKey("apikeytest1234567890");                     // APIリクエストエラー検証用 ※値書き換え不要
    Serial.println("[STORAGE] API key saved");

    ConfigData config;
    loadConfig(config);
    config.lat = 35.68037286903755;              // 東京駅の緯度（テスト用）
    config.lng = 139.76687900640945;             // 東京駅の経度（テスト用）
    // config.lat = 0;                              // ヌル島の緯度（機体0件テスト用）
    // config.lng = 0;                              // ヌル島の経度（機体0件テスト用）
    saveConfig(config);
    Serial.println("[STORAGE] Base Point saved.");
    
    updateBatteryLevel();
    Serial.println("[BOOT] updateBatteryLevel() done");     // 一時テストコード：電池アイコン見た目確認用

    struct tm timeInfo;
    if (syncTime(timeInfo)) {
        lastUpdateTime = formatUpdateTime(timeInfo);
    }

    // ここから計測（JSONパース）
    unsigned long startTime = millis();
    String rawJson;
    if (fetchFlightsRaw(rawJson)) {
        parseFlightsResponse(rawJson, foundFlights, totalFlightCount);
    }
    unsigned long elapsed = millis() - startTime;
    Serial.printf("[DATA] Fetch+Parse time: %lu ms, flights: %d\n", elapsed, totalFlightCount);
    // ここまで計測（JSONパース）

    // 画面遷移テスト用
    bool errorTest = false;                 // テストの内容に応じて書き換え

    if (errorTest) {
        // エラー画面の表示確認用
        currentError.message = "Missing api_key";
        currentError.code = "wrong_params";
        currentMode = MODE_ERROR_VIEW;
    } else {
        if (totalFlightCount > 0) {
            currentMode = MODE_FLIGHT_VIEW;
        } else {
            // 手順23実装により、機体0件画面へ直接遷移させる
            currentMode = MODE_NO_FLIGHTS_VIEW;
        }
    }
    needsRedraw = true;

    // ★★★一時テストコード↑ここまで

}

void loop() {
    M5.update();
    updateTouchButtons();                       // CoreS3など物理ボタンがない機種の仮想ボタンマッピング

    if (isApModeActive()) {
        handleCaptivePortal();
    }

    updateStateMachine();
}