#include <M5Unified.h>
#include <WiFi.h>
#include <DNSServer.h>
#include <WebServer.h>
#include <qrcode.h>
#include "wifi_handler.h"

/*
使用するQRコード描画ライブラリを変更したため、下記記述も書き換える
// QRコードをM5Stackの画面中央付近に描画する関数
void displayQRCode(const char* url) {
    // QRコードデータの生成（バージョン3・誤り訂正レベルLOW）
    QRCode qrcode;
    uint8_t qrcodeData[qrcode_getBufferSize(3)];
    qrcode_initText(&qrcode, qrcodeData, 3, ECC_LOW, url);

    // 描画サイズ・位置の計算（横方向中央揃え）
    int dotSize = 4;
    int xOffset = (320 - (qrcode.size * dotSize)) / 2;
    int yOffset = 100;

    // QRコード背景（クワイエットゾーン確保のための白塗り）
    M5.Lcd.fillRect(xOffset - 10, yOffset - 10, (qrcode.size * dotSize) + 20, (qrcode.size * dotSize) + 20, TFT_WHITE);

    // QRコード本体の描画（1マスずつ黒四角を配置）
    for (uint8_t y = 0; y < qrcode.size; y++) {
        for (uint8_t x = 0; x < qrcode.size; x++) {
            if (qrcode_getModule(&qrcode, x, y)) {
                M5.Lcd.fillRect(xOffset + (x * dotSize), yOffset + (y * dotSize), dotSize, dotSize, TFT_BLACK);
            }
        }
    }
}
*/

/*
WiFiManagerライブラリの使用を廃止したため、下記コードも書き換える
// WiFiManagerがAPモードに入ったときに呼ばれるコールバック関数
void configModeCallback(WiFiManager *myWiFiManager) {
    // 案内メッセージの画面表示（英語のみ・平易な表現）
    M5.Lcd.fillScreen(TFT_BLACK);
    M5.Lcd.setTextColor(TFT_WHITE);

    // タイトル（少し大きめ）
    M5.Lcd.setTextSize(2);
    M5.Lcd.setCursor(10, 10);
    M5.Lcd.println("Wi-Fi Setup");

    // 本文（フォントサイズを少し大きめに）
    M5.Lcd.setTextSize(2);
    M5.Lcd.setCursor(10, 45);
    M5.Lcd.setTextColor(TFT_WHITE);
    M5.Lcd.println("1. Connect your phone");
    M5.Lcd.println("   to Wi-Fi:");

    // SSID名を強調（黄色文字）
    M5.Lcd.setTextColor(TFT_YELLOW);
    M5.Lcd.println("   M5Stack-AP");

    // 通常表示に戻して続きを表示
    M5.Lcd.setTextColor(TFT_WHITE);
    M5.Lcd.println("");
    M5.Lcd.setCursor(10, M5.Lcd.getCursorY());          // 項番「1.～」と文頭を揃える
    M5.Lcd.println("2. Follow the");
    M5.Lcd.println("   instructions on");
    M5.Lcd.println("   the web page");

    // シリアルモニターへのログ出力
    Serial.println("Entered config mode");
    Serial.print("AP IP: ");
    Serial.println(WiFi.softAPIP());
}
*/

void setup() {
    // M5Stack本体の初期化
    // 起動の高速化および省電力化のため使用しない機能を明示的に無効化する
    auto cfg = M5.config();
    cfg.internal_imu = false;                           // 加速度センサー無効化
    cfg.internal_mic = false;                           // マイク無効化
    cfg.internal_spk = false;                           // スピーカー無効化（ブザー通知は現状使用しない想定）

    /*
        Wi-Fi関連処理よりも先に記述すること
        （CoreS3でウォッチドッグタイマーによるリセットがかかりフリーズする既知の現象を回避）
    */
    M5.begin(cfg);
    M5.Power.begin();

    initWiFi();

/*
WiFiManagerの使用は廃止
*/
    // // WiFiManagerのインスタンスを生成し、APモード時のコールバックを登録
    // WiFiManager wm;
    // wm.setAPCallback(configModeCallback);

    // // 保存済みのWi-Fi情報で自動接続を試行（失敗時はAPモードへ移行）
    // if (!wm.autoConnect("M5Stack-AP")) {
    //     Serial.println("Failed to connect and hit timeout");
    //     M5.Lcd.fillScreen(TFT_RED);
    //     M5.Lcd.setTextColor(TFT_WHITE);
    //     M5.Lcd.setCursor(10, 100);
    //     M5.Lcd.println("Connection Failed.");
    //     delay(3000);
    //     ESP.restart();
    // }

    // Wi-Fi接続成功時の画面表示
    M5.Lcd.fillScreen(TFT_BLACK);
    M5.Lcd.setTextColor(TFT_GREEN);
    M5.Lcd.setTextSize(2);
    M5.Lcd.setCursor(10, 50);
    M5.Lcd.println("Wi-Fi Connected!");
    M5.Lcd.setCursor(10, 90);
    M5.Lcd.setTextColor(TFT_WHITE);
    M5.Lcd.print("IP: ");
    M5.Lcd.println(WiFi.localIP());

    Serial.println("Wi-Fi Connected Successfully!");
}

void loop() {
    M5.update();

    if (isWiFiConnected())
    {
        /* TODO : 処理内容をここに記述 */
    }
    
    // 今後のステップ（ボタン押下時のAirLabs APIリクエスト等）をここに実装
}