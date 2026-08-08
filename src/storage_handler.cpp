/*
    保存処理の実装（NVS／LittleFS+JSON）
    書き込み、読み込み処理の戻り値は成否（bool）
*/

#include "storage_handler.h"
#include <Preferences.h>
#include <LittleFS.h>
#include <ArduinoJson.h>

// NVS名前空間ごとのPreferencesインスタンス
// Preferences...NVSへキー・バリュー形式でデータを読み書きするためのAPI
// ESP32標準ライブラリが提供するクラス）
Preferences wifiPrefs;                      // Wi-Fi資格情報
Preferences apiPrefs;                       // APIキー

// config.json / cache.json の保存先パス
static const char* CONFIG_PATH = "/config.json";
static const char* CACHE_PATH  = "/cache.json";

// ストレージ機能の初期化（LittleFSのマウントのみ。Preferencesは使用時にbegin/endする）
bool initStorage() {
    if (!LittleFS.begin()) {
        Serial.println("LittleFS mount failed");
        return false;
    }
    return true;
}

// ============================================================
// Wi-Fi資格情報（NVS、名前空間: "wifi"）
// ============================================================

// Wi-Fi資格情報保存
bool saveWifiCredentials(const String& ssid, const String& password) {
    wifiPrefs.begin("wifi", false);         // false = 読み書きモード
    wifiPrefs.putString("ssid", ssid);
    wifiPrefs.putString("password", password);
    wifiPrefs.end();
    return true;
}

// Wi-Fi資格情報読み込み
bool loadWifiCredentials(String& ssid, String& password) {
    wifiPrefs.begin("wifi", true);          // true = 読み取り専用モード
    ssid = wifiPrefs.getString("ssid", "");
    password = wifiPrefs.getString("password", "");
    wifiPrefs.end();

    // SSIDが空なら「未登録」と判定する
    // .getString()は未保存時にデフォルト値の空文字列を返すため文字数で判断
    return ssid.length() > 0;
}

// Wi-Fi資格情報消去
void clearWifiCredentials() {
    wifiPrefs.begin("wifi", false);
    wifiPrefs.clear();   // "wifi"名前空間を丸ごと消去
    wifiPrefs.end();
}

// ============================================================
// APIキー（NVS、名前空間: "api"）
// ============================================================

// APIキー保存
bool saveApiKey(const String& apiKey) {
    apiPrefs.begin("api", false);
    apiPrefs.putString("key", apiKey);
    apiPrefs.end();
    return true;
}

// APIキー読み込み
bool loadApiKey(String& apiKey) {
    apiPrefs.begin("api", true);
    apiKey = apiPrefs.getString("key", "");
    apiPrefs.end();

    // APIキーが空なら「未登録」と判定する
    return apiKey.length() > 0;
}

// APIキー消去
void clearApiKey() {
    apiPrefs.begin("api", false);
    apiPrefs.clear();
    apiPrefs.end();
}

// ============================================================
// 設定値：静的IP・取得地点・SCAN RANGE（LittleFS + JSON、config.json）
// ============================================================

// 設定情報保存
bool saveConfig(const ConfigData& config) {
    JsonDocument doc;
    doc["useStaticIp"] = config.useStaticIp;        // DHCP or StaticIP
    doc["staticIp"] = config.staticIp;
    doc["gateway"] = config.gateway;
    doc["subnet"] = config.subnet;
    doc["dns"] = config.dns;
    doc["lat"] = config.lat;                        // base point latitude
    doc["lng"] = config.lng;                        // base point longitude
    doc["scanRange"] = config.scanRange;            // NARROW or WIDE

    String newJson;
    serializeJson(doc, newJson);                    // 設定情報docをJSON形式文字列に整形後newJsonに書き出し

    // 既存ファイルと内容が同じであれば書き込みをスキップする（フラッシュ摩耗対策）
    // ESP32のフラッシュメモリには寿命がある（一般に10,000〜100,000回程度）
    // ※寿命は書き込みのみで読み込みには関係ない
    File existingFile = LittleFS.open(CONFIG_PATH, "r");
    String currentJson = existingFile ? existingFile.readString() : "";
    if (existingFile) existingFile.close();

    if (newJson == currentJson) {
        return true;
    }

    File file = LittleFS.open(CONFIG_PATH, "w");
    if (!file) {
        Serial.println("Failed to open config.json for writing");
        return false;
    }
    file.print(newJson);
    file.close();
    return true;
}

// 設定情報読み込み
bool loadConfig(ConfigData& config) {
    File file = LittleFS.open(CONFIG_PATH, "r");
    if (!file) {
        return false;                                           // 設定ファイルなし...初回起動時、設定クリア直後など
    }

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, file);    // JSONデータの解析（戻り値は成否）
    file.close();

    if (error) {
        Serial.println("Failed to parse config.json");
        return false;
    }

    // JSONから取得した値を構造体へセット
    // 「|」の右側はキーが存在しない or 型が一致しない場合のデフォルト値 ※ArduinoJsonライブラリ特有の記法
    config.useStaticIp = doc["useStaticIp"] | false;
    config.staticIp = doc["staticIp"] | "";
    config.gateway = doc["gateway"] | "";
    config.subnet = doc["subnet"] | "";
    config.dns = doc["dns"] | "";
    config.lat = doc["lat"] | 0.0;
    config.lng = doc["lng"] | 0.0;
    config.scanRange = doc["scanRange"] | "NARROW";

    return true;
}

void clearConfig() {
    if (LittleFS.exists(CONFIG_PATH)) {
        LittleFS.remove(CONFIG_PATH);
    }
}

// ============================================================
// 機体情報キャッシュ・残りリクエスト数（LittleFS + JSON、cache.json）
// ============================================================

// キャッシュ保存
bool saveCache(const FlightData flights[], int flightCount, int remainingRequests) {
    JsonDocument doc;
    doc["remainingRequests"] = remainingRequests;

    JsonArray flightArray = doc["flights"].to<JsonArray>();
    for (int i = 0; i < flightCount; i++) {
        JsonObject f = flightArray.add<JsonObject>();
        f["callsign"] = flights[i].callsign;
        f["flightIcao"] = flights[i].flightIcao;
        f["airlineIcao"] = flights[i].airlineIcao;
        f["dist"] = flights[i].dist;
        f["alt"] = flights[i].alt;
        f["from"] = flights[i].from;
        f["to"] = flights[i].to;
        f["type"] = flights[i].type;
        f["speed"] = flights[i].speed;
        f["heading"] = flights[i].heading;
        f["squawk"] = flights[i].squawk;
    }

    // cache.jsonは高頻度書き込みのため差分チェックは行わない
    File file = LittleFS.open(CACHE_PATH, "w");
    if (!file) {
        Serial.println("Failed to open cache.json for writing");
        return false;
    }
    serializeJson(doc, file);
    file.close();
    return true;
}

// キャッシュ読み込み
bool loadCache(FlightData flights[], int& flightCount, int& remainingRequests) {
    File file = LittleFS.open(CACHE_PATH, "r");
    if (!file) {
        flightCount = 0;
        return false;   // 未存在＝キャッシュなし
    }

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, file);
    file.close();

    if (error) {
        Serial.println("Failed to parse cache.json");
        flightCount = 0;
        return false;
    }

    remainingRequests = doc["remainingRequests"] | 0;

    JsonArray flightArray = doc["flights"].as<JsonArray>();
    flightCount = 0;
    for (JsonObject f : flightArray) {
        if (flightCount >= MAX_FLIGHT_COUNT) break;                 // FlightData配列の上限（flight_data.h参照）

        // 欠損時のデフォルト値
        // 文字列項目...空文字列、数値項目...0
        // ただし下記数値項目については0が実測値となるケースもありうるため、欠損時のデフォルト値を-1とする
        // alt, speed, heading
        flights[flightCount].callsign = f["callsign"] | "";
        flights[flightCount].flightIcao = f["flightIcao"] | "";
        flights[flightCount].airlineIcao = f["airlineIcao"] | "";
        flights[flightCount].dist = f["dist"] | 0.0;
        flights[flightCount].alt = f["alt"] | -1;
        flights[flightCount].from = f["from"] | "";
        flights[flightCount].to = f["to"] | "";
        flights[flightCount].type = f["type"] | "";
        flights[flightCount].speed = f["speed"] | -1;
        flights[flightCount].heading = f["heading"] | -1;
        flights[flightCount].squawk = f["squawk"] | "";

        flightCount++;
    }

    return true;
}

// キャッシュ消去
void clearCache() {
    if (LittleFS.exists(CACHE_PATH)) {
        LittleFS.remove(CACHE_PATH);
    }
}