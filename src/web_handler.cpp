/**
 * web_handler.cpp
 * Webサーバー・設定ページ配信関連の実装
 */

#include "web_handler.h"

#include <LittleFS.h>
#include <WebServer.h>
#include <WiFi.h>

#include "storage_handler.h"
#include "wifi_handler.h"

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
    // 1. リトライ回数カウンタのリセット
    //    新たな接続試行セッションの開始となるため、ここでリセットする
    // ------------------------------------------------------
    wifiRetryCount = 0;                         // 変数はwifi_handler.cppで定義

    // ------------------------------------------------------
    // 2. 必須項目（SSID）の受信・チェック
    // ------------------------------------------------------
    String ssid = server.arg("ssid");
    String password = server.arg("password");

    if (ssid.length() == 0) {
        server.send(400, "text/plain", "SSID is required.");
        return;
    }

    // ------------------------------------------------------
    // 3. 接続方式・静的IP項目の受信
    // ------------------------------------------------------
    String connType = server.arg("connType");           // "dhcp" または "static"
    bool useStaticIp = (connType == "static");

    String ipStr = server.arg("ip");
    String gatewayStr = server.arg("gateway");
    String subnetStr = server.arg("subnet");
    String dnsStr = server.arg("dns");                   // 空欄可（Gatewayを流用）

    // ------------------------------------------------------
    // 4. 静的IP選択時のみ、サーバー側バリデーションを実施
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
    // 5. Wi-Fi資格情報の保存（NVS）
    // ------------------------------------------------------
    saveWifiCredentials(ssid, password);

    // ------------------------------------------------------
    // 6. 静的IP設定の保存（config.json）
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
    // 7. レスポンス（装飾なしテキスト）
    // ------------------------------------------------------
    server.send(200, "text/plain", "Saved. Restarting...");
}