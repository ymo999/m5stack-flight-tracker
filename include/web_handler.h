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

#endif