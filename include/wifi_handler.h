/**
 * wifi_handler.h
 * Wi-Fi接続・APモード・キャプティブポータル関連の関数宣言
 */

#ifndef WIFI_HANDLER_H                          // インクルードガード（二重定義防止）
#define WIFI_HANDLER_H

#include <WebServer.h>

// Wi-Fi接続試行のタイムアウト（ミリ秒）
#define WIFI_CONNECT_TIMEOUT_MS 10000

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
void enterAPMode();

// キャプティブポータル関連の処理をloop()内で呼び出す
// ・DNSServer::processNextRequest()（DNS要求の処理）
// ・WebServer::handleClient()（HTTP要求の処理）
void handleCaptivePortal();

// キャプティブポータルを終了し、APモードを停止する
// ・DNSServer::stop()
// ・WebServer::stop()
// ・WiFi.softAPdisconnect(true)
void exitAPMode();

// 起動時のWi-Fi接続判定（登録あり→接続試行、未登録→APモード起動）
void initWiFi();

// 保存済みの資格情報・ネットワーク設定で、1回分の接続試行を行う
// 戻り値: true = 接続成功、false = 接続失敗
// RETRY・機体情報再取得時の再接続等、複数の呼び出し元から呼び出される想定
bool tryConnectWiFi();

bool isWiFiConnected();

bool isApModeActive();

// 保存されているWi-Fi資格情報・ネットワーク設定（静的IP関連）をクリアし、APモードへ移行する
// 異なるネットワークへの切り替えフロー専用（確認ダイアログでCONFIRMされた後、state_machine.cpp側から呼び出される想定）
void resetAndEnterAPMode();

// Wi-Fiを有効化する（ステーションモードへ切り替え。接続自体はtryConnectWiFi()等が別途行う）
void enableWiFi();

// Wi-Fiを無効化する（低消費電力運用のため、通信不要時はOFFにする）
void disableWiFi();

#endif