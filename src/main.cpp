#include <Arduino.h>
#include <DNSServer.h>
#include <M5Unified.h>
#include <WebServer.h>
#include <WiFi.h>

#include "api_handler.h"                // テストコード用
#include "flight_data.h"                // テストコード用
#include "state_machine.h"
#include "storage_handler.h"            // テストコード用
#include "system_status.h"              // テストコード用（電池アイコン見た目確認用）
#include "ui_handler.h"
#include "wifi_handler.h"

void setup() {

    // シリアル通信の初期化（ボーレートはplatformio.ini/モニタ側の設定と合わせる）※デバッグ用
    Serial.begin(115200);       Serial.println("[BOOT] Serial initialized");

    // M5Stack本体の初期化
    // 起動の高速化および省電力化のため使用しない機能を明示的に無効化する
    auto cfg = M5.config();
    cfg.internal_imu = false;                           // 加速度センサー無効化
    cfg.internal_mic = false;                           // マイク無効化
    cfg.internal_spk = false;                           // スピーカー無効化（ブザー通知は現状使用しない想定）

    // Wi-Fi関連処理よりも先に記述すること
    // （CoreS3でウォッチドッグタイマーによるリセットがかかりフリーズする既知の現象を回避）
    M5.begin(cfg);
    M5.Power.begin();           Serial.println("[BOOT] M5.begin() done");

    initStorage();              Serial.println("[BOOT] initStorage() done");
    initWiFi();                 Serial.println("[BOOT] initWiFi() done");
    initStateMachine();         Serial.println("[BOOT] initStateMachine() done");
    Serial.println("[BOOT] setup() complete");
    
    if (isWiFiConnected()) {
        
        // printfの%s書式は本来const char*を期待するので.c_str()を明示的に呼ぶ
        Serial.printf("[BOOT] Wi-Fi connected(%s)\n", WiFi.localIP().toString().c_str());

    }

    // 一時テストコード（起動時分岐ロジックが未実装のため、手順16の動作確認用に
    // 暫定的にここでAPIキー・基準地点を仮登録し、機体情報を取得してMODE_FLIGHT_VIEWへ遷移させる。
    // 本来はAPIキー・基準地点の登録状況に応じた分岐（QR_VIEW等）が必要。8.2-C実装時に置き換え予定）↓ここから
    // saveApiKey("YOUR_API_KEY_HERE");
    // Serial.println("API key saved.");

    // ConfigData config;
    // loadConfig(config);
    // config.lat = 35.68037286903755;              // 東京駅の緯度（テスト用）
    // config.lng = 139.76687900640945;             // 東京駅の経度（テスト用）
    // saveConfig(config);
    // Serial.println("Base Point saved.");
    
    // updateBatteryLevel();       Serial.println("[BOOT] updateBatteryLevel() done");     // 一時テストコード：電池アイコン見た目確認用

    // struct tm timeInfo;
    // if (syncTime(timeInfo)) {
    //     lastUpdateTime = formatUpdateTime(timeInfo);
    // }

    // // ここから計測（JSONパース）
    // unsigned long startTime = millis();
    // String rawJson;
    // if (fetchFlightsRaw(rawJson)) {
    //     parseFlightsResponse(rawJson, foundFlights, totalFlightCount);
    // }
    // unsigned long elapsed = millis() - startTime;
    // Serial.printf("Fetch+Parse time: %lu ms, flights: %d\n", elapsed, totalFlightCount);
    // // ここまで計測（JSONパース）

    // if (totalFlightCount > 0) {
    //     currentMode = MODE_FLIGHT_VIEW;
    //     needsRedraw = true;
    // }

    // 一時テストコード（手順16動作確認用）↑ここまで

}

void loop() {
    M5.update();

    if (isApModeActive()) {
        handleCaptivePortal();
    }

    updateStateMachine();
}