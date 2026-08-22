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
 * @brief APIキー・基準地点いずれかの保存が成功したかどうかを示すフラグを共有する
 *
 * @note QRコード誘導画面（MODE_QR_VIEW）で、保存完了を検知して自動的にSETTINGSへ
 *       戻るために使用する
 * @note APIキー専用にせず汎用の名前にしているのは、基準地点でも
 *       同じ仕組みを再利用する想定のため
 */
extern bool qrSetupCompleted;

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

/**
 * @brief 登録済みの基準地点をJSON形式で返す（GET "/location_current"）
 *
 * @note location.htmlが初期表示地点（東京駅／登録済みの値）を判定するために使用する
 * @note 未登録の場合はLOCATION_UNSET（storage_handler.h参照）がそのまま返る
 */
void handleLocationCurrent();

/**
 * @brief 基準地点設定ページ（location.html）を配信する（GET "/location"）
 */
void handleLocationPage();

/**
 * @brief 基準地点の受信・バリデーション・保存を行う（GET "/set"）
 *
 * @note 緯度は-90〜90、経度は-180〜180の範囲外の場合は失敗として扱う
 *       （この範囲チェックにより、センチネル値LOCATION_UNSET(9999)も自動的に弾かれる）
 * @note 検証結果に応じたHTMLページ（成功／失敗）を直接レスポンスとして返す
 */
void handleLocationSave();

#endif