/**
 * @file storage_handler.h
 * @brief NVS/LittleFSへの保存・読み込み関連の関数宣言
 */

#ifndef STORAGE_HANDLER_H           // インクルードガード（二重定義防止）
#define STORAGE_HANDLER_H

#include <Arduino.h>

#include "flight_data.h"            // FlightData構造体

/**
 * @brief ネットワーク設定・取得地点・SCAN RANGEをまとめた構造体（config.json対応）
 */
struct ConfigData {
    bool useStaticIp;               ///< true: 静的IP, false: DHCP
    String staticIp;                ///< M5Stack自身のIP
    String gateway;                 ///< ゲートウェイ
    String subnet;                  ///< サブネットマスク
    String dns;                     ///< DNSサーバー（未入力時はgatewayを流用）
    double lat;                     ///< 取得地点：緯度
    double lng;                     ///< 取得地点：経度
    String scanRange;               ///< "NARROW" または "WIDE"
};
 
/**
 * @brief ストレージ機能の初期化（Preferences・LittleFSのマウント）
 * 
 * @return マウント成否
 */ 
bool initStorage();

// ------------------------------------------------------
// Wi-Fi資格情報（NVS）
// ------------------------------------------------------

/**
 * @brief Wi-Fi資格情報（SSID・パスワード）をNVSへ保存する
 *
 * @param[in] ssid 保存するSSID
 * @param[in] password 保存するパスワード
 * @return 保存成否
 */
bool saveWifiCredentials(const String& ssid, const String& password);

/**
 * @brief Wi-Fi資格情報（SSID・パスワード）をNVSから読み込む
 *
 * @param[out] ssid 読み込んだSSIDの格納先
 * @param[out] password 読み込んだパスワードの格納先
 * @return true 登録あり（SSIDが空でない）
 * @return false 未登録（SSIDが空）
 */
bool loadWifiCredentials(String& ssid, String& password);

/**
 * @brief Wi-Fi資格情報のうち、SSIDのみをNVSから読み込む（CONFIG画面等での表示用）
 *
 * @note パスワードを取得しないことで、不要な機密情報のメモリ展開を避ける
 *
 * @param[out] ssid 読み込んだSSIDの格納先
 * @return true 登録あり（SSIDが空でない）
 * @return false 未登録（SSIDが空）
 */
bool loadWifiSsid(String& ssid);

/**
 * @brief Wi-Fi資格情報をNVSから消去する
 */
void clearWifiCredentials();

// ------------------------------------------------------
// APIキー（NVS）
// ------------------------------------------------------

/**
 * @brief APIキーをNVSへ保存する
 *
 * @param[in] apiKey 保存するAPIキー
 * @return 保存成否
 */
bool saveApiKey(const String& apiKey);

/**
 * @brief APIキーをNVSから読み込む
 *
 * @param[out] apiKey 読み込んだAPIキーの格納先
 * @return true 登録あり（apiKeyが空でない）
 * @return false 未登録（apiKeyが空）
 */
bool loadApiKey(String& apiKey);

/**
 * @brief APIキーをNVSから消去する
 */
void clearApiKey();

// ------------------------------------------------------
// 設定値：静的IP・取得地点・SCAN RANGE（LittleFS + JSON、config.json）
// ------------------------------------------------------

/**
 * @brief 設定値（静的IP・取得地点・SCAN RANGE）をLittleFS（config.json）へ保存する
 *
 * @note 既存ファイルと内容が同じ場合は書き込みをスキップする（フラッシュ摩耗対策）
 *
 * @param[in] config 保存する設定値
 * @return 保存成否
 */
bool saveConfig(const ConfigData& config);

/**
 * @brief 設定値（静的IP・取得地点・SCAN RANGE）をLittleFS（config.json）から読み込む
 *
 * @note 読み込みに失敗した場合も、configには安全なデフォルト値がセットされる（呼び出し側の初期化保証に依存しない）
 *
 * @param[out] config 読み込んだ設定値の格納先
 * @return true 読み込み成功
 * @return false 失敗（ファイル未存在またはパース失敗。configにはデフォルト値がセットされる）
 */
bool loadConfig(ConfigData& config);

/**
 * @brief 設定値（静的IP・取得地点・SCAN RANGE）を消去する（config.jsonファイル自体を削除）
 *
 * @note 関数名が類似しているclearNetworkConfig()（静的IP項目のみを消去する別関数）と混同しないこと
 * RESET ALL専用
 */
void clearConfig();

/**
 * @brief ネットワーク関連項目（静的IP設定）のみをクリアする
 * 
 * 取得地点（lat/lng）・SCAN RANGEは既存の値を維持したまま保存し直す
 * 
 * @note 関数名が類似しているclearConfig()（config.json全体を削除する、RESET ALL専用の別関数）と混同しないこと
 * ネットワーク切り替えフロー（異なるWi-Fiへの再設定）専用
 */
void clearNetworkConfig();

// ------------------------------------------------------
// 機体情報キャッシュ・残りリクエスト数（LittleFS + JSON、cache.json）
// ------------------------------------------------------

/**
 * @brief 機体情報キャッシュ・残リクエスト数・最終取得日時をLittleFS（cache.json）へ保存する
 *
 * @note 両者を1ファイルに統合することで、書き込み回数を削減している（フラッシュ摩耗対策）
 *
 * @param[in] flights 機体データの配列
 * @param[in] flightCount 機体数
 * @param[in] remainingRequests 残リクエスト数
 * @param[in] lastUpdateTime 取得日時（MM/DD HH:MM形式）
 * @return 保存成否
 */
bool saveCache(const FlightData flights[], int flightCount, int remainingRequests, const String& lastUpdateTime);

/**
 * @brief 機体情報キャッシュ・残リクエスト数・最終取得日時をLittleFS（cache.json）から読み込む
 *
 * @param[out] flights 機体データの格納先配列
 * @param[out] flightCount 機体数の格納先
 * @param[out] remainingRequests 残リクエスト数の格納先
 * @param[out] lastUpdateTime 取得日時の格納先（未保存時は"--/-- --:--"）
 * @return true 読み込み成功
 * @return false 失敗（ファイル未存在またはパース失敗）
 */
bool loadCache(FlightData flights[], int& flightCount, int& remainingRequests, String& lastUpdateTime);

/**
 * @brief 残りリクエスト数のみをLittleFS（cache.json）から読み込む（CONFIG画面等での表示用）
 *
 * @note 機体データ配列（FlightData[MAX_FLIGHT_COUNT]）の展開を避けるため、remainingRequestsのみ読み込む
 *
 * @param[out] remainingRequests 残リクエスト数の格納先
 * @return true 読み込み成功
 * @return false 失敗（ファイル未存在またはパース失敗）
 */
bool loadRemainingRequests(int& remainingRequests);

/**
 * @brief 機体情報キャッシュ・残りリクエスト数を消去する（cache.jsonファイル自体を削除）
 */
void clearCache();

#endif