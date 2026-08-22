#include <Arduino.h>
#include <DNSServer.h>
#include <M5Unified.h>
#include <WebServer.h>
#include <WiFi.h>

#include "flight_data.h"
#include "input_handler.h"
#include "state_machine.h"
#include "storage_handler.h"
#include "system_status.h"              // 物理ボタンを持たない機種にチャタリング防止措置を適用させるため
#include "ui_handler.h"                 // 接続中画面の描画（drawLoadingScreen()）のため
#include "web_handler.h"
#include "wifi_handler.h"

void setup() {

    // シリアル通信の初期化（ボーレートはplatformio.ini/モニタ側の設定と合わせる）※デバッグ用
    Serial.begin(115200);
    
    // ===== 調査用一時コード（ここから） =====
    // USB CDC接続の確立を待つための待機
    // モニターが安定接続する前のログ欠落を防ぐための調査用処置
    // USB未接続時にはMAX秒数待機してしまうため、デバッグ時以外はコメントアウトすること

    // ミリ秒単位で時間を測定
    uint32_t serialWaitStartTime = millis();
    
    // 「シリアル未準備」かつ「開始から指定秒数以内」の間ループする
    while (!Serial && (millis() - serialWaitStartTime < 5000)) {
        delay(100);                         // 接続を待つ
    }
    // ===== 調査用一時コード（ここまで） =====

    Serial.println("[BOOT] Serial initialized");

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
    Serial.println("[BOOT] M5.begin() done");

    // ボタンデバウンスを30msに設定（チャタリング防止）
    // 物理ボタンを持たない機種のみ対象（デフォルト値10msだとタッチのノイズを拾いやすいため）
    // ※Basic等、物理ボタンのある機種はデフォルトのまま維持する
    if (isVirtualButtonBoard())
    {
        M5.BtnA.setDebounceThresh(30);
        M5.BtnB.setDebounceThresh(30);
        M5.BtnC.setDebounceThresh(30);
    }
    
    initStorage();
    Serial.println("[BOOT] initStorage() done");

    // 起動時にキャッシュを復元する
    // （接続失敗画面のボタン出し分け等でtotalFlightCountを参照するため）
    int remainingRequests = 0;
    bool hasCache = loadCache(foundFlights, totalFlightCount, remainingRequests, lastUpdateTime);
    Serial.printf("[BOOT] loadCache() done. hasCache: %d, flights: %d, remainingRequests: %d\n", hasCache, totalFlightCount, remainingRequests);

    // Wi-Fi接続には最大60秒かかるため、処理中であることを画面に示す
    // （再起動を挟む方式のため、この間に描画がないと無反応に見えてしまう）
    drawLoadingScreen("Connecting to Wi-Fi...");

    initWiFi();

    /* Wi-Fi接続失敗検証用テストコード ※後で必ず戻すこと
       ※ここでは誤った情報を上書きしているだけ（Wi-Fi設定自体は「initWifi()」で終わっている）
       誤った情報による接続エラー動作を確認したい場合は、この後
       ・FLIGHT_VIEWの最終機体→NEXT→再取得
       または
       ・再起動
       を行うこと
    */
    // ↓ ここから
    // String currentSsid;
    // loadWifiSsid(currentSsid);
    // saveWifiCredentials(currentSsid, "wrong_password_for_test");
    // ↑ ここまで
    Serial.println("[BOOT] initWiFi() done");

    initStateMachine(hasCache);
    Serial.println("[BOOT] initStateMachine() done");

    updateBatteryLevel();
    Serial.println("[BOOT] updateBatteryLevel() done");

    Serial.println("[BOOT] setup() complete");
    
    if (isWiFiConnected()) {        
        Serial.print("[BOOT] Wi-Fi connected(");
        Serial.print(WiFi.localIP().toString());
        Serial.println(")");
    }
}

void loop() {
    M5.update();
    updateTouchButtons();                       // CoreS3など物理ボタンがない機種の仮想ボタンマッピング

    if (isApModeActive()) {
        handleCaptivePortal();
    }

    if (isConfigServerActive()) {
        server.handleClient();
    }

    updateStateMachine();
}