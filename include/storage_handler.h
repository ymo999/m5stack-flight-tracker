/*
    NVS/LittleFSへの保存・読み込み関連の関数宣言
*/
#ifndef STORAGE_HANDLER_H           // インクルードガード（二重定義防止）
#define STORAGE_HANDLER_H

#include <Arduino.h>
#include "flight_data.h"            // FlightData構造体

// ネットワーク設定・取得地点・SCAN RANGEをまとめた構造体（config.json対応）
struct ConfigData {
    bool useStaticIp;               // true: 静的IP, false: DHCP
    String staticIp;                // M5Stack自身のIP
    String gateway;                 // ゲートウェイ
    String subnet;                  // サブネットマスク
    String dns;                     // DNSサーバー（未入力時はgatewayを流用）
    double lat;                     // 取得地点：緯度
    double lng;                     // 取得地点：経度
    String scanRange;               // "NARROW" または "WIDE"
};

// ストレージ機能の初期化（Preferences・LittleFSのマウント）
bool initStorage();

// Wi-Fi資格情報（NVS）
bool saveWifiCredentials(const String& ssid, const String& password);
bool loadWifiCredentials(String& ssid, String& password);
void clearWifiCredentials();

// APIキー（NVS）
bool saveApiKey(const String& apiKey);
bool loadApiKey(String& apiKey);
void clearApiKey();

// 設定値：静的IP・取得地点・SCAN RANGE（LittleFS + JSON、config.json）
bool saveConfig(const ConfigData& config);
bool loadConfig(ConfigData& config);
void clearConfig();

// 機体情報キャッシュ・残りリクエスト数（LittleFS + JSON、cache.json）
bool saveCache(const FlightData flights[], int flightCount, int remainingRequests);
bool loadCache(FlightData flights[], int& flightCount, int& remainingRequests);
void clearCache();

#endif