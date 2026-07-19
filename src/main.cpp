#include <M5Stack.h>
#include <WiFi.h>
#include <DNSServer.h>
#include <WebServer.h>
#include <WiFiManager.h>
#include <qrcode.h>

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

// WiFiManagerがAPモードに入ったときに呼ばれるコールバック関数
void configModeCallback(WiFiManager *myWiFiManager) {

    /* TODO
     * QRコード表示しなくても、WiFiManagerにより「Wi-Fi: M5Stack-AP」に接続するだけでWiFi設定は可能
     * QRコードは表示せず（関数は他で使用するので残す） 
     * 案内メッセージも
     * 「Wi-Fi: M5Stack-AP」に接続してあとはWebページの指示に従うように
     * の旨の文章に変更（フォントサイズ少し大きめにした方がよいかも）
     */


    // 案内メッセージの画面表示（英語のみ・平易な表現）
    M5.Lcd.fillScreen(TFT_BLACK);
    M5.Lcd.setTextColor(TFT_WHITE);

    // タイトル（少し大きめ）
    M5.Lcd.setTextSize(2);
    M5.Lcd.setCursor(10, 10);
    M5.Lcd.println("Wi-Fi Setup");

    // 本文
    M5.Lcd.setTextSize(1);
    M5.Lcd.setCursor(10, 35);
    M5.Lcd.println("1. Connect your phone to");
    M5.Lcd.println("   Wi-Fi: M5Stack-AP");
    M5.Lcd.setCursor(10, M5.Lcd.getCursorY());          // 項番「1.～」と左揃えにする
    M5.Lcd.println("2. Scan the QR code below");
    M5.Lcd.println("   or open this address:");
    M5.Lcd.println("   http://192.168.4.1");

    // 設定用URLの組み立てとQRコード表示
    String configUrl = "http://" + WiFi.softAPIP().toString();
    displayQRCode(configUrl.c_str());

    // シリアルモニターへのログ出力
    Serial.println("Entered config mode");
    Serial.print("AP IP: ");
    Serial.println(WiFi.softAPIP());
}

void setup() {
    // M5Stack本体の初期化
    M5.begin();
    M5.Power.begin();

    // 起動時の画面表示
    M5.Lcd.fillScreen(TFT_BLACK);
    M5.Lcd.setTextColor(TFT_GREEN);
    M5.Lcd.setTextSize(2);
    M5.Lcd.println("Starting Wi-Fi Tracker...");

    // WiFiManagerのインスタンスを生成し、APモード時のコールバックを登録
    WiFiManager wm;
    wm.setAPCallback(configModeCallback);

    // 保存済みのWi-Fi情報で自動接続を試行（失敗時はAPモードへ移行）
    if (!wm.autoConnect("M5Stack-AP")) {
        Serial.println("Failed to connect and hit timeout");
        M5.Lcd.fillScreen(TFT_RED);
        M5.Lcd.setTextColor(TFT_WHITE);
        M5.Lcd.setCursor(10, 100);
        M5.Lcd.println("Connection Failed.");
        delay(3000);
        ESP.restart();
    }

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
    // 今後のステップ（ボタン押下時のAirLabs APIリクエスト等）をここに実装
}