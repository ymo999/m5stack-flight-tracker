/**
 * wifi_handler.h
 * Wi-Fi接続・APモード・キャプティブポータル関連の関数宣言
 */

#ifndef WIFI_HANDLER_H                          // インクルードガード（二重定義防止）
#define WIFI_HANDLER_H

#include <WebServer.h>

// wifi_handler.cpp で定義されているWebServerインスタンスを共有する
extern WebServer server;

// Wi-Fi接続失敗時のリトライ回数カウンタ
// 実体はwifi_handler.cppに定義。web_handler.cpp（/save受信時のリセット）と共有する
extern int wifiRetryCount;

// APモードを起動し、キャプティブポータル（DNSServer + 302リダイレクト）を開始する
// ・WiFi.softAP() でAPを起動
// ・DNSServer で全ドメイン問い合わせに自機IPを返す
// ・"/" と "/save" のルーティングを登録（中身は web_handler 側、手順7で実装）
// ・未登録パスは "/" へ302リダイレクト
void startCaptivePortal();

// キャプティブポータル関連の処理をloop()内で呼び出す
// ・DNSServer::processNextRequest()（DNS要求の処理）
// ・WebServer::handleClient()（HTTP要求の処理）
void handleCaptivePortal();

// キャプティブポータルを終了し、APモードを停止する
// ・DNSServer::stop()
// ・WebServer::stop()
// ・WiFi.softAPdisconnect(true)
void stopCaptivePortal();

// 関数宣言
void initWiFi();
void handleWiFiSetup();
bool isWiFiConnected();

#endif