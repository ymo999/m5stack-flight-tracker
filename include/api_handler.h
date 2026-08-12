/**
 * api_handler.h
 * API通信関連の関数宣言
 */

#ifndef API_HANDLER_H                   // インクルードガード（二重定義防止）
#define API_HANDLER_H

#include <Arduino.h>

#include "flight_data.h"

// AirLabs APIへ周辺機体情報のリクエストを送信し、生のJSONレスポンスを取得する
// Bounding Box（緯度経度範囲）は、保存済みの取得地点・SCAN RANGE（storage_handler経由）から内部で計算する
// 戻り値: true = リクエスト成功（HTTPステータス200）、false = 失敗
bool fetchFlightsRaw(String& responsePayload);

// AirLabs APIの生JSONレスポンスをパースし、FlightData配列に格納する
// 基準地点からの距離順（近い順）に、最大MAX_FLIGHT_COUNT件を保持する（超過分は遠い機体から捨てる）
// 距離計算に必要な基準地点（lat/lng）は、内部でstorage_handler経由で読み込む
void parseFlightsResponse(const String& rawJson, FlightData flights[], int& flightCount);

// NTPで日本標準時に同期する
// 呼び出し時点でWi-Fi接続済みであることが前提
// 戻り値: true = 同期成功（timeInfoに結果を格納）、false = 同期失敗
bool syncTime(struct tm& timeInfo);

#endif