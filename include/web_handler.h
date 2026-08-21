/**
 * @file web_handler.h
 * @brief Webサーバー・設定ページ配信関連の関数宣言
 */

#ifndef WEB_HANDLER_H                               // インクルードガード（二重定義防止）
#define WEB_HANDLER_H

/**
 * @brief 初回設定ページ（wifi.html）を配信する（GET "/"）
 */
void handleSetupPage();

/**
 * @brief フォーム送信内容を受信し、バリデーション後に保存する（POST "/save"）
 *
 * @note SSID未入力、または静的IP選択時にIP/Gateway/Subnet/DNSのいずれかが不正な場合は400エラーを返す
 * @note 保存成功時はWi-Fi資格情報（NVS）・静的IP設定（config.json）を保存した上で、
 * ESP.restart()により再起動し、保存済み設定での接続を試行する
 */
void handleSave();

// ------------------------------------------------------
// 設定用Webサーバー（stationモード）― APIキー設定ページ
// ------------------------------------------------------

/**
 * @brief 設定用Webサーバー（stationモード）を起動する
 *
 * APIキー設定ページ（"/api_key"）のGET/POSTルートを登録し、server.begin()する
 *
 * @note 基準地点設定ページ（"/location"）のルートは手順28で追加する
 * @note APモード用のenterAPMode()とは、同じserverインスタンスを別タイミングで
 *       使い分ける関係にある（station接続中はAPモードではないため、同時起動はしない前提）
 */
void startConfigServer();

/**
 * @brief 設定用Webサーバー（stationモード）を停止する
 */
void stopConfigServer();

/**
 * @brief 設定用Webサーバーが起動中かどうかを返す
 *
 * @note apModeActive/isApModeActive()と同じパターン
 */
bool isConfigServerActive();

/**
 * @brief APIキー設定ページ（api_key.html）を配信する（GET "/api_key"）
 */
void handleApiKeyPage();

/**
 * @brief APIキー送信を受信し、ping検証後にNVSへ保存する（POST "/api_key"）
 *
 * @note 検証結果に応じたHTMLページ（成功／失敗）を直接レスポンスとして返す
 * @note validateApiKey()はapi_handler.h/.cppに実装
 */
void handleApiKeySave();

#endif