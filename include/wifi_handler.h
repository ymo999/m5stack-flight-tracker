/**
 * @file wifi_handler.h
 * @brief Wi-Fi接続・APモード・キャプティブポータル関連の関数宣言
 */

#ifndef WIFI_HANDLER_H                          // インクルードガード（二重定義防止）
#define WIFI_HANDLER_H

#include <WebServer.h>

/**
 * @def WIFI_CONNECT_TIMEOUT_MS
 * @brief Wi-Fi接続試行のタイムアウト（ミリ秒）
 *
 * リトライ機構を持たず1回のみの試行とするため、スマートフォンの回線切り替え時間を見込んで長めに確保
 */
#define WIFI_CONNECT_TIMEOUT_MS 60000

/**
 * @brief wifi_handler.cppで定義されているWebServerインスタンスを共有する
 */
extern WebServer server;

/**
 * @brief APモードを起動し、キャプティブポータル（DNSServer + 302リダイレクト）を開始する
 *
 * ・WiFi.softAP()でAPを起動
 * ・DNSServerで全ドメイン問い合わせに自機IPを返す
 * ・"/"と"/save"のルーティングを登録（中身はweb_handler側、手順7で実装）
 * ・未登録パスは"/"へ302リダイレクト
 */
void enterAPMode();

/**
 * @brief キャプティブポータル関連の処理をloop()内で呼び出す
 *
 * ・DNSServer::processNextRequest()（DNS要求の処理）
 * ・WebServer::handleClient()（HTTP要求の処理）
 */
void handleCaptivePortal();

/**
 * @brief キャプティブポータルを終了し、APモードを停止する
 *
 * ・DNSServer::stop()
 * ・WebServer::stop()
 * ・WiFi.softAPdisconnect(true)
 */
void exitAPMode();

/**
 * @brief 起動時のWi-Fi接続判定（登録あり→接続試行、未登録→APモード起動）
 */
void initWiFi();

/**
 * @brief 保存済みの資格情報・ネットワーク設定で、1回分の接続試行を行う
 *
 * @return true 接続成功
 * @return false 接続失敗
 */
bool tryConnectWiFi();

/**
 * @brief 現在Wi-Fiに接続中かどうかを返す
 *
 * @return true 接続中
 * @return false 未接続
 */
bool isWiFiConnected();

/**
 * @brief 現在APモードが起動中かどうかを返す
 *
 * @return true APモード起動中
 * @return false APモード起動していない
 */
bool isApModeActive();

/**
 * @brief 保存されているWi-Fi資格情報・ネットワーク設定（静的IP関連）をクリアし、APモードへ移行する
 *
 * @note 異なるネットワークへの切り替えフロー専用
 * （確認ダイアログでCONFIRMされた後、state_machine.cpp側から呼び出される想定）
 */
void resetAndEnterAPMode();

/**
 * @brief Wi-Fiを有効化する（ステーションモードへ切り替え）
 *
 * @note 接続自体はtryConnectWiFi()等が別途行う
 */
void enableWiFi();

/**
 * @brief Wi-Fiを無効化する
 * 
 * （低消費電力運用のため、通信不要時はOFFにする）
 */
void disableWiFi();

#endif