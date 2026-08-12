/**
 * api_handler.cpp
 * API通信関連の実装
 */

#include "api_handler.h"

#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <TinyGPSPlus.h>
#include <WiFiClientSecure.h>

#include "storage_handler.h"

// AirLabs APIのベースURL
static const char* API_BASE_URL = "https://airlabs.co/api/v9/flights";

// SCAN RANGEに対応するマージン（度）。2.1参照
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
        Serial.print("AirLabs API request failed, HTTP code: ");
        Serial.println(httpCode);
    }

    http.end();
    return success;
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
void parseFlightsResponse(const String& rawJson, FlightData flights[], int& flightCount) {
    flightCount = 0;

    // ------------------------------------------------------
    // 1. 距離計算に使う基準地点を読み込む
    // ------------------------------------------------------
    ConfigData config;
    loadConfig(config);

    // ------------------------------------------------------
    // 2. JSON文字列をパースする
    // ------------------------------------------------------
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, rawJson);
    if (error) {
        Serial.println("Failed to parse flights response");
        return;
    }

    JsonArray responseArray = doc["response"].as<JsonArray>();
    if (responseArray.isNull()) {
        Serial.println("Flights response has no 'response' array");
        return;
    }

    // ------------------------------------------------------
    // 3. 各機体を1件ずつ処理し、距離順で上位MAX_FLIGHT_COUNT件を保持する
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

        // 欠損値センチネル（-1）。2.3節・flight_data.h参照
        data.alt     = flight["alt"] | -1;
        data.speed   = flight["speed"] | -1;
        data.heading = flight["dir"] | -1;

        // 距離計算（lat/lngが欠けている場合は計算不能のためスキップ）→切り捨てる
        if (!flight["lat"].is<double>() || !flight["lng"].is<double>()) {
            continue;
        }
        double lat = flight["lat"];
        double lng = flight["lng"];
        data.dist = TinyGPSPlus::distanceBetween(config.lat, config.lng, lat, lng) / 1000.0;

        insertFlightByDistance(flights, flightCount, data);
    }
}