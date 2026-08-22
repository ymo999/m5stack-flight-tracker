/**
 * storage_handler.cpp
 * 保存処理の実装（NVS／LittleFS+JSON）
 * 書き込み、読み込み処理の戻り値は成否（bool）
 * 秘匿性が高いものはNVS、それ以外のものはLittleFS+JSONへ書き込む
 * ※本来ならばNVS暗号化＆フラッシュ暗号化で秘匿性を担保すべきだが、
 * 要件上そこまでのセキュリティは求めていないので、単純にNVSへの書き込みのみとする
*/

#include "storage_handler.h"

#include <ArduinoJson.h>
#include <LittleFS.h>
#include <Preferences.h>

/**
 * NVS名前空間ごとのPreferencesインスタンス
 * Preferences...NVSへキー・バリュー形式でデータを読み書きするためのAPI
 * （ESP32標準ライブラリが提供するクラス）
 * 消去のタイミングが異なるため、名前空間を別にして丸ごと消去できるようにしている
 */
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

// Wi-Fi資格情報読み込み（SSIDのみ、CONFIG画面等での表示用）
// パスワードを取得しないことで、不要な機密情報のメモリ展開を避ける
bool loadWifiSsid(String& ssid) {
    wifiPrefs.begin("wifi", true);          // true = 読み取り専用モード
    ssid = wifiPrefs.getString("ssid", "");
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

    // 初期化（ファイルが存在しない場合や、読み込みに失敗した場合に不正値が残らないようにする
    config.useStaticIp = false;
    config.staticIp = "";
    config.gateway = "";
    config.subnet = "";
    config.dns = "";
    config.lat = LOCATION_UNSET;
    config.lng = LOCATION_UNSET;
    config.scanRange = "NARROW";

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
    config.lat = doc["lat"] | LOCATION_UNSET;
    config.lng = doc["lng"] | LOCATION_UNSET;
    config.scanRange = doc["scanRange"] | "NARROW";

    return true;
}

// 設定情報全クリア（RESET ALL用）
void clearConfig() {
    if (LittleFS.exists(CONFIG_PATH)) {
        LittleFS.remove(CONFIG_PATH);
    }
}

// ネットワーク関連項目（静的IP設定）のみをクリアする
// 取得地点（lat/lng）・SCAN RANGEは既存の値を維持したまま保存し直す
void clearNetworkConfig() {
    ConfigData config;
    loadConfig(config);             // 未登録時は空のConfigDataのまま（lat/lng/scanRangeもデフォルト値）

    // 静的IP関連の項目のみデフォルト値に戻す
    config.useStaticIp = false;
    config.staticIp = "";
    config.gateway = "";
    config.subnet = "";
    config.dns = "";

    // lat/lng/scanRangeはloadConfig()で読み込んだ値のまま変更しないため、
    // saveConfig()で書き戻しても既存の取得地点・SCAN RANGEは保持される
    saveConfig(config);
}

// ============================================================
// 機体情報キャッシュ・残りリクエスト数（LittleFS + JSON、cache.json）
// ============================================================

/**
 * キャッシュ保存（レスポンスJSONを解析して配列にセットされた値をキャッシュに保存する）
 * 残リクエスト数も同じタイミングでキャッシュに保存する（書き込み回数低減のため）
 * データコピーの手間を軽減するため配列はポインタで受け取る
 * この関数では配列の値を読み取るだけで書き換えないため、const修飾子を付加している（loadCacheは書き込みを行うため付加しない）
*/
bool saveCache(const FlightData flights[], int flightCount, int remainingRequests, const String& lastUpdateTime) {
    JsonDocument doc;
    doc["remainingRequests"] = remainingRequests;
    doc["lastUpdateTime"] = lastUpdateTime;

    // flightCount（機体数）の分ループし、機体データ（ポインタ経由で参照）をJSON配列に1機ずつセット
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

    // cache.jsonは差分チェックは行わない（毎回差分が発生するため差分チェックの意味がない）
    File file = LittleFS.open(CACHE_PATH, "w");
    if (!file) {
        Serial.println("Failed to open cache.json for writing");
        return false;
    }
    serializeJson(doc, file);
    file.close();
    return true;
}

/**
 * キャッシュ読み込み（キャッシュから取り出した値を、呼び出し元の配列にセットする）
 * 配列はポインタ、件数・残リクエスト数は参照で受け取ることでコピーの手間を防ぐ
 * 残リクエスト数は、書き込み回数低減のためキャッシュと同時に記録している
 */
bool loadCache(FlightData flights[], int& flightCount, int& remainingRequests, String& lastUpdateTime) {
    File file = LittleFS.open(CACHE_PATH, "r");
    if (!file) {
        flightCount = 0;
        lastUpdateTime = "--/-- --:--";
        return false;                                               // JSONファイルなし＝キャッシュなし
    }

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, file);
    file.close();

    if (error) {
        Serial.println("Failed to parse cache.json");
        flightCount = 0;
        lastUpdateTime = "--/-- --:--";
        return false;
    }

    remainingRequests = doc["remainingRequests"] | 0;
    lastUpdateTime = doc["lastUpdateTime"] | "--/-- --:--";

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

// 残りリクエスト数のみ読み込む（CONFIG画面等での表示用）
// 機体データ配列（FlightData[MAX_FLIGHT_COUNT]）の展開を避けるため、
// remainingRequestsフィールドのみをパースする
bool loadRemainingRequests(int& remainingRequests) {
    File file = LittleFS.open(CACHE_PATH, "r");
    if (!file) {
        remainingRequests = 0;
        return false;   // 未存在＝キャッシュなし
    }

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, file);
    file.close();

    if (error) {
        Serial.println("Failed to parse cache.json");
        remainingRequests = 0;
        return false;
    }

    remainingRequests = doc["remainingRequests"] | 0;

    return true;
}

// キャッシュ消去
void clearCache() {
    if (LittleFS.exists(CACHE_PATH)) {
        LittleFS.remove(CACHE_PATH);
    }
}