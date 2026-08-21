/**
 * web_handler.cpp
 * Webサーバー・設定ページ配信関連の実装
 */

#include "web_handler.h"

#include <LittleFS.h>
#include <WebServer.h>
#include <WiFi.h>

#include "storage_handler.h"
#include "ui_handler.h"                                 // 再起動予告画面の描画のため
#include "wifi_handler.h"
#include "api_handler.h"                                // validateApiKey()でAPIキーの有効性を検証するため

// 設定用Webサーバー（stationモード）が起動中かどうかを示すフラグの実体
// apModeActiveと同じパターンで管理する
bool configServerActive = false;

// ============================================================
// 初回設定ページの配信（GET "/"）
// ============================================================
void handleSetupPage() {
    File file = LittleFS.open("/wifi.html", "r");
    if (!file) {
        server.send(500, "text/plain", "Failed to load setup page");
        return;
    }
    server.streamFile(file, "text/html");
    file.close();
}

// ============================================================
// フォーム受信・バリデーション・保存（POST "/save"）
// ============================================================
void handleSave() {

    // ------------------------------------------------------
    // 1. 必須項目（SSID）の受信・チェック
    // ------------------------------------------------------
    String ssid = server.arg("ssid");
    String password = server.arg("password");

    if (ssid.length() == 0) {
        server.send(400, "text/plain", "SSID is required.");
        return;
    }

    // ------------------------------------------------------
    // 2. 接続方式・静的IP項目の受信
    // ------------------------------------------------------
    String connType = server.arg("connType");               // "dhcp" または "static"
    bool useStaticIp = (connType == "static");

    String ipStr = server.arg("ip");
    String gatewayStr = server.arg("gateway");
    String subnetStr = server.arg("subnet");
    String dnsStr = server.arg("dns");                      // 空欄可（Gatewayを流用）

    // ------------------------------------------------------
    // 3. 静的IP選択時のみ、サーバー側バリデーションを実施
    // ------------------------------------------------------
    IPAddress ip, gateway, subnet, dns;

    if (useStaticIp) {
        // IP／Gateway／Subnetは必須。1つでも不正なら400エラー
        if (!ip.fromString(ipStr) || !gateway.fromString(gatewayStr) || !subnet.fromString(subnetStr)) {
            server.send(400, "text/plain", "Invalid IP, Gateway, or Subnet.");
            return;
        }

        // DNSは空欄ならGatewayを流用。入力されている場合のみ形式チェック
        if (dnsStr.length() == 0) {
            dns = gateway;
            dnsStr = gatewayStr;                        // config.json保存用の文字列側も揃える
        } else if (!dns.fromString(dnsStr)) {
            server.send(400, "text/plain", "Invalid DNS.");
            return;
        }
    }

    // ------------------------------------------------------
    // 4. Wi-Fi資格情報の保存（NVS）
    // ------------------------------------------------------
    saveWifiCredentials(ssid, password);

    // ------------------------------------------------------
    // 5. 静的IP設定の保存（config.json）
    //    既存の取得地点・SCAN RANGE設定を保持するため、
    //    読み込み→静的IP項目のみ上書き→保存の手順を踏む
    // ------------------------------------------------------
    ConfigData config;
    loadConfig(config);                                 // 未登録時は空のConfigDataのまま

    config.useStaticIp = useStaticIp;
    config.staticIp = ipStr;
    config.gateway = gatewayStr;
    config.subnet = subnetStr;
    config.dns = dnsStr;

    saveConfig(config);

    // ------------------------------------------------------
    // 6. レスポンス（装飾なしテキスト）
    // ------------------------------------------------------
    server.send(200, "text/plain", "Saved. Restarting...");

    // ------------------------------------------------------
    // 7. 再起動して接続試行
    //      setup()を再実行することによりinitWiFi() → tryConnectWiFi()を
    //      保存済み資格情報・静的IP設定を使用して実行させる
    //      （RESET ALL実装時と同じ設計方針：ESP.restart()による再起動を採用）
    //      本体画面には再起動の予告を表示する（表示秒数とdelay()を一致させること）
    //      このdelay()は、レスポンス送信の完了待ち（送信途中での切断防止）も兼ねる
    // ------------------------------------------------------
    drawLoadingScreen("Restarting in 3 seconds");
    delay(3000);                                        // レスポンス送信の完了を待つ（送信途中での切断を防ぐ）
    ESP.restart();
}

// ============================================================
// 設定用Webサーバー（stationモード）の起動
// ============================================================
void startConfigServer() {
    server.on("/api_key", HTTP_GET, handleApiKeyPage);
    server.on("/api_key", HTTP_POST, handleApiKeySave);
    // 基準地点設定ページ（"/location"）のルートは手順28で追加する

    server.begin();
    configServerActive = true;
}

// ============================================================
// 設定用Webサーバー（stationモード）の停止
// ============================================================
void stopConfigServer() {
    server.stop();
    configServerActive = false;
}

bool isConfigServerActive() {
    return configServerActive;
}

// ------------------------------------------------------
// LittleFS上のHTMLファイルをストリーム配信する共通処理
// handleApiKeyPage()・handleApiKeySave()の両方から使用する
// ------------------------------------------------------
static void streamHtmlFile(const char* path) {
    File file = LittleFS.open(path, "r");
    if (!file) {
        server.send(500, "text/plain", "Failed to load page");
        return;
    }
    server.streamFile(file, "text/html");
    file.close();
}

// ============================================================
// APIキー設定ページの配信（GET "/api_key"）
// ============================================================
void handleApiKeyPage() {
    streamHtmlFile("/api_key.html");
}

// ============================================================
// APIキーの受信・ping検証・保存（POST "/api_key"）
// ============================================================
void handleApiKeySave() {
    // ------------------------------------------------------
    // 1. 必須項目（ユーザーが登録したAPIキー）の受信・チェック
    // ------------------------------------------------------
    String apiKey = server.arg("apikey");

    if (apiKey.length() == 0) {
        server.send(400, "text/plain", "API key is required.");
        return;
    }

    // ------------------------------------------------------
    // 2. ping検証（有効な場合のみNVSへ保存）
    // ------------------------------------------------------
    if (validateApiKey(apiKey)) {
        saveApiKey(apiKey);
        streamHtmlFile("/api_key_success.html");
    } else {
        streamHtmlFile("/api_key_error.html");
    }
}