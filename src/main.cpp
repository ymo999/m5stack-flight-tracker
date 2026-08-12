#include <M5Unified.h>
#include <WiFi.h>
#include <DNSServer.h>
#include <WebServer.h>
#include <qrcode.h>

#include "api_handler.h"                // テストコード用
#include "flight_data.h"                // テストコード用
#include "storage_handler.h"            // テストコード用
#include "wifi_handler.h"

void setup() {

    // シリアル通信の初期化（ボーレートはplatformio.ini/モニタ側の設定と合わせる）
    Serial.begin(115200);

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

    initStorage();
    initWiFi();
    
    if (isWiFiConnected()) {
        
        // 一時テストコード（手順12確認用、動作確認は完了済み。8.2-C実装時に本来のUI表示へ置き換え予定）↓ここから
        // // Wi-Fi接続成功時の画面表示
        // M5.Lcd.fillScreen(TFT_BLACK);
        // M5.Lcd.setTextColor(TFT_GREEN);
        // M5.Lcd.setTextSize(2);
        // M5.Lcd.setCursor(10, 50);
        // M5.Lcd.println("Wi-Fi Connected!");
        // M5.Lcd.setCursor(10, 90);
        // M5.Lcd.setTextColor(TFT_WHITE);
        // M5.Lcd.print("IP: ");
        // M5.Lcd.println(WiFi.localIP());

        // Serial.println("Wi-Fi Connected Successfully!");
        // 一時テストコード（手順12確認用）↑ここまで

    } else {
        handleCaptivePortal();   // ← 追加：APモード中はDNS・HTTP要求を処理する
    }

    // 一時テストコード（手順27・28未実装のための代替。本来はapi_key.html/location.htmlで行う想定）↓ここから
    // saveApiKey("YOUR_API_KEY_HERE");
    // Serial.println("API key saved.");

    // ConfigData config;
    // loadConfig(config);
    // config.lat = 35.68037286903755;
    // config.lng = 139.76687900640945;
    // saveConfig(config);
    // Serial.println("Base Point saved.");

    // // ここから計測
    // unsigned long startTime = millis();
    // String rawJson;
    // if (fetchFlightsRaw(rawJson)) {
    //     parseFlightsResponse(rawJson, foundFlights, totalFlightCount);
    // }
    // unsigned long elapsed = millis() - startTime;
    // Serial.printf("Fetch+Parse time: %lu ms, flights: %d\n", elapsed, totalFlightCount);
    // 一時テストコード（手順11・12動作確認用）↑ここまで

}

void loop() {
    M5.update();

    // if (isWiFiConnected()) {
    //     /* TODO : 処理内容をここに記述 */
    // } else {
    //     handleCaptivePortal();   // ← 追加：APモード中はDNS・HTTP要求を処理する
    // }

    // 今後のステップ（ボタン押下時のAirLabs APIリクエスト等）をここに実装
}