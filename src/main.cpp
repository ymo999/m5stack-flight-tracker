#include <Arduino.h>
#include <DNSServer.h>
#include <M5Unified.h>
#include <WebServer.h>
#include <WiFi.h>

#include "api_handler.h"                // テストコード用（APIリクエスト、JSONパース）
#include "error_data.h"                 // テストコード用（エラー画面確認用）
#include "flight_data.h"
#include "input_handler.h"
#include "secrets.h"                    // テストコード用（AIRLABS_API_KEY）※Git管理外
#include "state_machine.h"
#include "storage_handler.h"
#include "system_status.h"              // 物理ボタンを持たない機種にチャタリング防止措置を適用させるため
#include "ui_handler.h"                 // テストコード用（最終更新日時整形）
#include "wifi_handler.h"

void setup() {

    // シリアル通信の初期化（ボーレートはplatformio.ini/モニタ側の設定と合わせる）※デバッグ用
    Serial.begin(115200);
    
    // ===== 調査用一時コード（ここから） =====
    // USB CDC接続の確立を待つための待機
    // モニターが安定接続する前のログ欠落を防ぐための調査用処置
    // USB未接続時にはMAXの3秒待機してしまうため、デバッグ時以外はコメントアウトすること

    // ミリ秒単位で時間を測定
    uint32_t serialWaitStartTime = millis();
    
    // 「シリアル未準備」かつ「開始から3秒以内」の間ループする
    while (!Serial && (millis() - serialWaitStartTime < 3000)) {
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
    loadCache(foundFlights, totalFlightCount, remainingRequests);
    Serial.printf("[BOOT] loadCache() done. flights: %d, remainingRequests: %d\n", totalFlightCount, remainingRequests);

    // Wi-Fi接続には最大60秒かかるため、処理中であることを画面に示す
    // （再起動を挟む方式のため、この間に描画がないと無反応に見えてしまう）
    drawLoadingScreen("Connecting to Wi-Fi...");

    initWiFi();
    // Wi-Fi接続失敗検証用 ※後で必ず戻すこと ↓ ここから
    // String currentSsid;
    // loadWifiSsid(currentSsid);
    // saveWifiCredentials(currentSsid, "wrong_password_for_test");
    // ↑ ここまで
    Serial.println("[BOOT] initWiFi() done");

    initStateMachine();
    Serial.println("[BOOT] initStateMachine() done");

    updateBatteryLevel();
    Serial.println("[BOOT] updateBatteryLevel() done");

    Serial.println("[BOOT] setup() complete");
    
    if (isWiFiConnected()) {        
        Serial.print("[BOOT] Wi-Fi connected(");
        Serial.print(WiFi.localIP().toString());
        Serial.println(")");
    }

    // ★★★一時テストコード↓ここから
    // ※APIキー・基準地点の登録状況による起動時分岐は8.2-Dで実装予定のため、暫定的に
    // 　ここでAPIキー・基準地点を仮登録し、機体情報を取得してMODE_FLIGHT_VIEWへ遷移させる
    // ※initStateMachine()はWi-Fi接続成功時にcurrentModeを変更しないため、このブロック内で
    // 　必ずcurrentModeを設定すること（設定しないとMODE_WIFI_SETUPのまま残り、
    // 　意図せず接続失敗画面が描画される）
    // ※Wi-Fi未接続時はinitStateMachine()が既にMODE_WIFI_SETUPを設定しているため、
    // 　このブロック全体を実行しない（実行するとその判定結果を上書きしてしまう）
    if (isWiFiConnected()) {

        // APIキーの設定（実装までの代替措置）
        saveApiKey(AIRLABS_API_KEY);
        // saveApiKey("apikeytest1234567890");                     // APIリクエストエラー検証用 ※値書き換え不要
        Serial.println("[STORAGE] API key saved");

        // 基準地点の設定（実装までの代替措置）
        ConfigData config;
        loadConfig(config);
        config.lat = 35.68037286903755;              // 東京駅の緯度（テスト用）
        config.lng = 139.76687900640945;             // 東京駅の経度（テスト用）
        // config.lat = 0;                              // ヌル島の緯度（機体0件テスト用）
        // config.lng = 0;                              // ヌル島の経度（機体0件テスト用）
        saveConfig(config);
        Serial.println("[STORAGE] Base Point saved");

        // 最終更新日時の取得と整形
        struct tm timeInfo;
        if (syncTime(timeInfo)) {
            lastUpdateTime = formatUpdateTime(timeInfo);
        }

        // APIリクエスト送信・JSONパース        
        unsigned long startTime = millis();                         // ここから計測（JSONパース）
        String rawJson;
        int testRemainingRequests = 0;                              // 一時テストコード用に定義
        ErrorData parseError;                                       // 一時テストコード用に定義
        if (fetchFlightsRaw(rawJson)) {
            Serial.println("[BOOT] fetchFlightsRaw() done");
            if (parseFlightsResponse(rawJson, foundFlights, totalFlightCount, testRemainingRequests, parseError))
            {
                unsigned long elapsed = millis() - startTime;       // ここまで計測（JSONパース）
                Serial.printf("[BOOT] Fetch+Parse time: %lu ms, flights: %d, remainingRequests: %d\n", elapsed, totalFlightCount, testRemainingRequests);
            } else {
                Serial.println("[BOOT] parseFlightsResponse() failed");
            }
        } else {
            Serial.println("[BOOT] fetchFlightsRaw() failed");
        }

        // 画面遷移テスト用
        bool errorTest = false;                                     // テストの内容に応じてtrue/false切替

        // 次画面遷移
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
    }
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