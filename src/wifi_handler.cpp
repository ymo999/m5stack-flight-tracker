/**
 * wifi_handler.cpp
 * Wi-Fi接続・APモード・キャプティブポータル関連機能
 */

#include "wifi_handler.h"

#include <DNSServer.h>
#include <WebServer.h>
#include <WiFi.h>

#include "secrets.h"                                // AP_SSID、AP_PASSWORD（Git管理外）
#include "storage_handler.h"
#include "web_handler.h"

// APモード・キャプティブポータルで使用するインスタンス
DNSServer dnsServer;
// web_handler.cppと共有するため、wifi_handler.hでextern宣言している
WebServer server(80);                               // HTTPのみ

// APモードが現在起動中かどうかを示すフラグの実体
// enterAPMode()でtrue、exitAPMode()でfalseに設定する
bool apModeActive = false;

// ============================================================
// APモード＋キャプティブポータルの起動
// ============================================================
void enterAPMode() {

    // APモード移行
    // APモードへ移行する前に静的IP設定をクリアする
    // （直前まで静的IP接続していた場合、内部ルーティングテーブルが競合するため）
    WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE);

    // 戻り値で起動成否を判定する（secrets.hの書き換えミス等、実行時の異常検知のため）
    bool apStarted = WiFi.softAP(AP_SSID, AP_PASSWORD);
    if (!apStarted) {
        // Serial.println("Failed to start AP mode (check secrets.h)");
    }

    // 全ドメインへのDNS問い合わせには自機のIPアドレスを返す
    dnsServer.start(53, "*", WiFi.softAPIP());

    // 初回設定ページの配信・フォーム受信処理を登録（処理の実装はweb_handler側）
    server.on("/", handleSetupPage);                // 設定情報入力ページ
    server.on("/save", HTTP_POST, handleSave);      // 登録ボタン押下後

    // 未登録パス（OSごとの独自の検証用URL）は、すべてルートパスの設定ページ（wifi.html）に強制リダイレクト（302）
    server.onNotFound([](){
        String targetURL = "http://" + WiFi.softAPIP().toString() + "/";        // 自機IPのURL生成
        server.sendHeader("Location", targetURL, true);
        server.send(302, "text/plain", "");
    });

    server.begin();

    apModeActive = true;
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
void exitAPMode() {
    dnsServer.stop();                               // ポート53番を閉じstart()で確保したリソースを解放
    server.stop();                                  // ポート80番を閉じる
    WiFi.softAPdisconnect(true);                    // APモード終了

    apModeActive = false;
}

// ============================================================
// 起動時のWi-Fi接続判定（登録あり→接続試行、未登録→APモード起動）
// ============================================================
void initWiFi() {
    String ssid, password;

    if (loadWifiCredentials(ssid, password)) {
        // 登録あり：保存済み情報で接続試行
        tryConnectWiFi();
    } else {
        // 未登録：APモードへ移行
        //  ※移行前の静的IP設定クリアは、そもそもこの時点で
        //  　接続試行そのものを行っていないため、WiFi.config()による
        //  　クリアは不要（静的IPが適用されるのはtryConnectWiFi()内のみ）
        enterAPMode();
    }
}

// ============================================================
// 保存済みの資格情報・ネットワーク設定で、1回分の接続試行を行う
// ============================================================
bool tryConnectWiFi() {
    // ------------------------------------------------------
    // 1. 保存済みの資格情報・ネットワーク設定を読み込む
    // ------------------------------------------------------
    String ssid, password;
    loadWifiCredentials(ssid, password);

    ConfigData config;
    loadConfig(config);

    // ------------------------------------------------------
    // 2. 静的IP選択時は、WiFi.begin()より前に適用する
    // ------------------------------------------------------
    if (config.useStaticIp) {
        IPAddress ip, gateway, subnet, dns;
        ip.fromString(config.staticIp);
        gateway.fromString(config.gateway);
        subnet.fromString(config.subnet);
        dns.fromString(config.dns);

        WiFi.config(ip, gateway, subnet, dns);
    }

    // ------------------------------------------------------
    // 3. 接続試行
    // ※ Arduino独自の文字列クラスであるStringを、C言語形式の文字列（const char*）に変換
    // ------------------------------------------------------
    WiFi.begin(ssid.c_str(), password.c_str());

    // ------------------------------------------------------
    // 4. タイムアウトまでポーリングして待つ
    // ------------------------------------------------------
    unsigned long startTime = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startTime < WIFI_CONNECT_TIMEOUT_MS) {
        delay(100);
    }

    // Serial.printf("[WIFI] WiFi.status() after attempt = %d (ssid=\"%s\")\n", WiFi.status(), ssid.c_str());
    bool connected = (WiFi.status() == WL_CONNECTED);

    // ------------------------------------------------------
    // 5. 接続成功時、静的IPならDNSを再適用する
    //    WiFi.config()で静的IPを設定してからWiFi.begin()で接続しても、
    //    DNS情報が正しく反映されない場合があるという既知の不具合への対処
    // ------------------------------------------------------
    if (connected && config.useStaticIp) {
        IPAddress ip, gateway, subnet, dns;
        ip.fromString(config.staticIp);
        gateway.fromString(config.gateway);
        subnet.fromString(config.subnet);
        dns.fromString(config.dns);

        WiFi.config(ip, gateway, subnet, dns);
        delay(100);   // DNS設定の反映を待つ
    }

    // ------------------------------------------------------
    // 6. 成否を返す
    // ------------------------------------------------------
    return connected;
}

bool isWiFiConnected() {
    return WiFi.status() == WL_CONNECTED;
}

bool isApModeActive() {
    return apModeActive;
}

// ============================================================
// 異なるネットワークへの切り替え
// ============================================================
/**
 * 保存されているWi-Fi資格情報・ネットワーク設定（静的IP関連）をクリアし、APモードへ移行する
 * 確認ダイアログでCONFIRMされた後に呼び出される想定（呼び出し元はstate_machine.cpp）
 */
void resetAndEnterAPMode() {
    // 1. NVSのWi-Fi資格情報をクリア
    clearWifiCredentials();

    // 2. ネットワーク設定（静的IP関連）のみクリア。取得地点・SCAN RANGEは維持
    clearNetworkConfig();

    // 3. 実行中のIP設定クリア＋APモードへ移行
    //    enterAPMode()冒頭でWiFi.config()による実行中IP設定のクリアも行われるため、ここでの個別呼び出しは不要
    enterAPMode();
}

// ============================================================
// Wi-Fi電源管理（低消費電力運用）
// ============================================================
// Wi-Fiを有効化する（ステーションモードへ切り替え）
// 接続自体はtryConnectWiFi()等の呼び出し元が別途行う
void enableWiFi() {
    WiFi.mode(WIFI_STA);
}

// Wi-Fiを無効化する
// 通信不要時はOFFにし、低消費電力運用を行う
void disableWiFi() {
    WiFi.mode(WIFI_OFF);
}