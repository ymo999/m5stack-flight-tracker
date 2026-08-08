/*
    wifi_handler.cpp
    Wi-Fi接続・APモード・キャプティブポータル関連機能
*/
#include <DNSServer.h>
#include <WebServer.h>
#include "wifi_handler.h"

DNSServer dnsServer;
WebServer server(80);                               // HTTPのみ

// プロトタイプ宣言
void startCaptivePortal();
void initWiFi();
void handleWiFiSetup();
bool isWiFiConnected();

void setup() {
  initWiFi();
}

void loop() {
    dnsServer.processNextRequest();                 // DNS要求の処理（ループ内で必要）
    server.handleClient();                          // HTTP要求の処理
}

// キャプティブポータル機能
void startCaptivePortal() {
    WiFi.softAP("M5Stack-AP");                      // アクセスポイント名 : M5Stack-AP

    // 全ドメインへのDNS問い合わせには自機のIPアドレスを返す
    dnsServer.start(53, "*", WiFi.softAPIP());

    // TODO : handleSetupPageとhandleSaveの定義
    /*
    server.on("/", handleSetupPage);
    server.on("/save", HTTP_POST, handleSave);
    */
    
    // 未登録パス（OSごとの独自の検証用URL）は、すべてルートパスの設定ページに強制リダイレクト（302）
    server.onNotFound([](){
        String targetURL = "http://" + WiFi.softAPIP().toString() + "/";        // 自機IPのURL生成
        server.sendHeader("Location", targetURL, true);
        server.send(302, "text/plain", "");
    });

    server.begin();
}

void initWiFi() {
    /* TODO : 処理内容の記述 */
}

void handleWiFiSetup() {
    /* TODO : 処理内容の記述 */
}

bool isWiFiConnected() {
    return WiFi.status() == WL_CONNECTED;
}