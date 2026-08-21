/**
 * api_handler.cpp
 * API通信関連の実装
 */

#include "api_handler.h"

#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <math.h>
#include <time.h>
#include <WiFiClientSecure.h>

#include "storage_handler.h"

// AirLabs APIのベースURL
static const char* API_BASE_URL = "https://airlabs.co/api/v9/flights";

// AirLabs APIのpingエンドポイント（APIキー検証用）
static const char* PING_BASE_URL = "https://airlabs.co/api/v9/ping";

// SCAN RANGEに対応するマージン（度）
static const double MARGIN_NARROW = 0.5;
static const double MARGIN_WIDE   = 2.0;

// ============================================================
// AirLabs APIへ周辺機体情報のリクエストを送信し、生のJSONレスポンスを取得する
// ============================================================
bool fetchFlightsRaw(String& responsePayload) {

    // ------------------------------------------------------
    // 1. 保存済みの取得地点・SCAN RANGE・APIキーを読み込む
    // ------------------------------------------------------
    ConfigData config;
    loadConfig(config);

    String apiKey;
    loadApiKey(apiKey);

    // ------------------------------------------------------
    // 2. SCAN RANGEに応じたマージンから、Bounding Boxを計算する
    // ------------------------------------------------------
    double margin = (config.scanRange == "WIDE") ? MARGIN_WIDE : MARGIN_NARROW;

    double lamin = config.lat - margin;
    double lamax = config.lat + margin;
    double lomin = config.lng - margin;
    double lomax = config.lng + margin;

    // ------------------------------------------------------
    // 3. リクエストURLを組み立てる（bbox順序: lamin,lomin,lamax,lomax）
    // ------------------------------------------------------
    String url = String(API_BASE_URL)
        + "?api_key=" + apiKey
        + "&bbox=" + String(lamin, 6) + "," + String(lomin, 6) + "," + String(lamax, 6) + "," + String(lomax, 6)
        + "&_fields=flight_icao,flight_iata,airline_icao,aircraft_icao,dep_iata,arr_iata,lat,lng,alt,dir,speed,squawk"
        + "&lang=en";

    // ------------------------------------------------------
    // 4. HTTPSリクエストを送信する
    //  　※証明書検証はスキップする（setInsecure()）。
    //  　　理由：AirLabsは独自ルートCA証明書を公式配布しておらず、setCACert()による
    //  　　検証は現実的でない。扱うデータ（航空機位置情報）の機密性も低いため、
    //  　　中間者攻撃耐性を犠牲にしてでも簡素な実装を優先する判断とした
    // ------------------------------------------------------
    WiFiClientSecure client;
    client.setInsecure();

    HTTPClient http;
    http.begin(client, url);

    int httpCode = http.GET();
    bool success = (httpCode == 200);

    if (success) {
        responsePayload = http.getString();
    } else {
        Serial.print("[API] AirLabs API request failed, HTTP code: ");
        Serial.println(httpCode);
    }

    http.end();
    return success;
}

// ============================================================
// AirLabsのpingエンドポイントを使い、APIキーの有効性を検証する
// ============================================================
bool validateApiKey(const String& apiKey) {
    String url = String(PING_BASE_URL) + "?api_key=" + apiKey;

    // ------------------------------------------------------
    // HTTPSリクエストを送信する
    // ※証明書検証はスキップする（fetchFlightsRaw()と同じ理由・同じ方針）
    // ------------------------------------------------------
    WiFiClientSecure client;
    client.setInsecure();

    HTTPClient http;
    http.begin(client, url);
    int httpCode = http.GET();

    // 通信自体が失敗した場合（httpCode <= 0）は無効として扱う
    if (httpCode <= 0) {
        http.end();
        return false;
    }

    String payload = http.getString();
    http.end();

    // ------------------------------------------------------
    // response.responseが"pong"かどうかのみで判定する
    // HTTPステータスコードには依存しない（AirLabsはエラー時も200を返すことがあるため）
    // ------------------------------------------------------
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, payload);
    if (error) {
        return false;
    }

    const char* responseValue = doc["response"] | "";
    return strcmp(responseValue, "pong") == 0;
}

// ============================================================
// 距離順ソート補助（挿入ソート方式）
// ============================================================
// 距離順を維持したまま、新しい機体データを配列に挿入する
// 配列が満杯の場合、現在の最遠機体より近ければ入れ替える。そうでなければ何もしない（捨てる）
static void insertFlightByDistance(FlightData flights[], int& flightCount, const FlightData& newFlight) {

    // 枠に空きがある場合：挿入位置を探して要素をずらす
    if (flightCount < MAX_FLIGHT_COUNT) {
        int insertPos = flightCount;
        while (insertPos > 0 && flights[insertPos - 1].dist > newFlight.dist) {
            flights[insertPos] = flights[insertPos - 1];
            insertPos--;
        }
        // 新しい機体データを所定の位置に上書きし、配列の機体数をカウントアップ
        flights[insertPos] = newFlight;
        flightCount++;
    }
    
    // 枠が満杯：現在の最遠機体より近ければ、末尾を追い出して挿入し直す
    else if (newFlight.dist < flights[MAX_FLIGHT_COUNT - 1].dist) {
        int insertPos = MAX_FLIGHT_COUNT - 1;
        while (insertPos > 0 && flights[insertPos - 1].dist > newFlight.dist) {
            flights[insertPos] = flights[insertPos - 1];
            insertPos--;
        }
        // 新しい機体データを所定の位置に上書きし、配列の機体数は既に満杯なのでカウントアップはしない
        flights[insertPos] = newFlight;
    }

    // else: 枠が満杯、かつ現在の最遠機体より遠い場合は何もしない
}

// ============================================================
// AirLabs APIの生JSONレスポンスをパースし、FlightData配列に格納する
// ============================================================
bool parseFlightsResponse(const String& rawJson, FlightData flights[], int& flightCount,
                          int& remainingRequests, ErrorData& errorOut) {
    flightCount = 0;
    remainingRequests = 0;

    // ------------------------------------------------------
    // 1. 距離計算に使う基準地点を読み込む
    // ------------------------------------------------------
    ConfigData config;
    loadConfig(config);

    // ------------------------------------------------------
    // 2. JSON文字列をパースする（パースエラーのメッセージはオリジナル）
    // ------------------------------------------------------
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, rawJson);
    if (error) {
        Serial.println("[JSON] Failed to parse flights response");
        errorOut.message = "JSON parse failed";
        errorOut.code = "parse_error";
        return false;
    }

    // ------------------------------------------------------
    // 3. AirLabs APIレベルのエラーを検出する
    //    例：{"error":{"message":"Missing api_key","code":"wrong_params"}}
    //    レスポンスのerror.message/error.codeをそのまま表示する方針のため、加工せず転記する
    // ------------------------------------------------------
    if (doc["error"].is<JsonObject>()) {
        errorOut.message = (const char*)(doc["error"]["message"] | "Unknown error");
        errorOut.code = (const char*)(doc["error"]["code"] | "unknown_error");
        return false;
    }

    // ------------------------------------------------------
    // 4. 残りリクエスト数を取得する（request.key.limits_total）
    //    キーが存在しない想定外レスポンスの場合は0のまま処理を継続する
    // ------------------------------------------------------
    remainingRequests = doc["request"]["key"]["limits_total"] | 0;

    // ------------------------------------------------------
    // 5. 機体情報配列を取得する（存在しない想定外形式は、エラーではなく0件として扱う）
    // ------------------------------------------------------
    JsonArray responseArray = doc["response"].as<JsonArray>();
    if (responseArray.isNull()) {
        Serial.println("[JSON] Flights response has no 'response' array");
        return true;
    }

    // ------------------------------------------------------
    // 6. 各機体を1件ずつ処理し、距離順で上位MAX_FLIGHT_COUNT件を保持する
    // ------------------------------------------------------
    for (JsonObject flight : responseArray) {
        FlightData data;

        // 便名（flight_icaoを優先、なければflight_iataにフォールバック）
        const char* flightIcao = flight["flight_icao"] | "";
        const char* flightIata = flight["flight_iata"] | "";
        data.flightIcao = flightIcao;
        data.callsign = (strlen(flightIcao) > 0) ? flightIcao : flightIata;

        data.airlineIcao = (const char*)(flight["airline_icao"] | "");
        data.type        = (const char*)(flight["aircraft_icao"] | "");
        data.from        = (const char*)(flight["dep_iata"] | "");
        data.to           = (const char*)(flight["arr_iata"] | "");
        data.squawk       = (const char*)(flight["squawk"] | "");

        // 欠損値センチネル（-1）※「0」が意味を持つ項目のため欠損を「-1」で表す
        data.alt     = flight["alt"] | -1;
        data.speed   = flight["speed"] | -1;
        data.heading = flight["dir"] | -1;

        // 距離計算（lat/lngが欠けている場合は計算不能のためスキップ）→切り捨てる
        if (!flight["lat"].is<double>() || !flight["lng"].is<double>()) {
            continue;
        }
        double lat = flight["lat"];
        double lng = flight["lng"];
        data.dist = calculateDistanceMeters(config.lat, config.lng, lat, lng) / 1000.0;

        insertFlightByDistance(flights, flightCount, data);
    }

    return true;
}

// ============================================================
// NTPで日本標準時に同期する
// ============================================================
bool syncTime(struct tm& timeInfo) {
    configTime(9 * 3600, 0, "ntp.nict.jp");     // JST（UTC+9）でNTP同期
    return getLocalTime(&timeInfo);             // timeInfoのアドレスに取得した現在日時を書き込み、取得成否を返す
}

// ============================================================
// 2点間の地表距離を計算する（単位：メートル、Haversine公式）
// ============================================================
double calculateDistanceMeters(double lat1, double lng1, double lat2, double lng2) {

    const double earthRadiusM = 6371000.0;      // 地球の平均半径（メートル）

    double lat1Rad = radians(lat1);
    double lat2Rad = radians(lat2);
    double deltaLatRad = radians(lat2 - lat1);
    double deltaLngRad = radians(lng2 - lng1);

    double a = sin(deltaLatRad / 2) * sin(deltaLatRad / 2)
             + cos(lat1Rad) * cos(lat2Rad) * sin(deltaLngRad / 2) * sin(deltaLngRad / 2);
    double c = 2 * atan2(sqrt(a), sqrt(1 - a));

    return earthRadiusM * c;
}