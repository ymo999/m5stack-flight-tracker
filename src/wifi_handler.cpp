/*
    wifi_handler.cpp
    Wi-Fi接続・APモード・キャプティブポータル関連機能
*/
#include <WiFi.h>
#include <DNSServer.h>
#include <WebServer.h>
#include "wifi_handler.h"

// APモード・キャプティブポータルで使用するインスタンス
DNSServer dnsServer;
WebServer server(80);                               // HTTPのみ

// ============================================================
// APモード＋キャプティブポータルの起動
// ============================================================
void startCaptivePortal() {
    WiFi.softAP("M5Stack-AP");                      // アクセスポイント名 : M5Stack-AP

    // 全ドメインへのDNS問い合わせには自機のIPアドレスを返す
    dnsServer.start(53, "*", WiFi.softAPIP());

    // TODO : handleSetupPageとhandleSaveの定義（web_handler側、手順7で実装）
    /*
    server.on("/", handleSetupPage);
    server.on("/save", HTTP_POST, handleSave);
    */

    // 未登録パス（OSごとの独自の検証用URL）は、すべてルートパスの設定ページ（wifi.html）に強制リダイレクト（302）
    server.onNotFound([](){
        String targetURL = "http://" + WiFi.softAPIP().toString() + "/";        // 自機IPのURL生成
        server.sendHeader("Location", targetURL, true);
        server.send(302, "text/plain", "");
    });

    server.begin();
}

// ============================================================
// キャプティブポータル関連処理（loop()から呼び出す）
// ============================================================
/**
 * dnsServer.processNextRequest()
 *  1. DNS要求が届いているか確認
 *  2. 届いていたらdnsServer.start()で渡した対象ドメインに一致するかチェック（全ドメイン"*"を対象にするので常にtrue）
 *  3. 一致していたらdnsServer.start()で渡したIPアドレスを返す
 * 
 * server.handleClient()
 *  1. HTTP要求が届いているか確認
 *  2. 届いていたらserver.on()で登録したパス（"/"、"/save"等）に一致するかチェック
 *  3. 一致すれば対応するハンドラを実行、一致しなければonNotFound()のハンドラを実行
 *      "/" ... フォーム画面（wifi.html）を表示（GET）
 *      "/save" ... フォームに入力した値を受け取り、保存・接続処理実行（POST）
 *      不一致 ... onNotFound() -> すべてルートパスの設定ページ（wifi.html）に強制リダイレクト（302）
 */
void handleCaptivePortal() {
    dnsServer.processNextRequest();                 // DNS要求の処理
    server.handleClient();                          // HTTP要求の処理
}

// ============================================================
// APモード・キャプティブポータルの終了
// ============================================================
void stopCaptivePortal() {
    dnsServer.stop();                               // ポート53番を閉じstart()で確保したリソースを解放
    server.stop();                                  // ポート80番を閉じる
    WiFi.softAPdisconnect(true);                    // APモード終了
}

// ============================================================
// 以下、他手順（手順8等）で実装予定の関数（現状はTODOのまま維持）
// ============================================================
void initWiFi() {
    /* TODO : 処理内容の記述 */
}

void handleWiFiSetup() {
    /* TODO : 処理内容の記述 */
}

bool isWiFiConnected() {
    return WiFi.status() == WL_CONNECTED;
}