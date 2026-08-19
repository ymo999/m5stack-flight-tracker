/**
 * @file api_handler.h
 * @brief API通信関連の関数宣言
 */

#ifndef API_HANDLER_H                   // インクルードガード（二重定義防止）
#define API_HANDLER_H

#include <Arduino.h>

#include "error_data.h"                 // ErrorData（AirLabs APIエラー・JSON解析失敗時の出力用）
#include "flight_data.h"

/**
 * @brief APIへ周辺機体情報のリクエストを送信し、生のJSONレスポンスを取得する
 * 
 * Bounding Box（緯度経度範囲）は、保存済みの取得地点・SCAN RANGE（storage_handler経由）から内部で計算する
 * 
 * @param[out] responsePayload （成功時のみ）AirLabsから返ってきた生のJSON文字列（HTTPレスポンスボディそのもの）格納先
 * @return true リクエスト成功（HTTPステータス200）
 * @return false 失敗
 */
bool fetchFlightsRaw(String& responsePayload);

/**
 * @brief APIの生JSONレスポンスをパースし、FlightData配列に格納する
 * 
 * 基準地点からの距離順（近い順）に、最大MAX_FLIGHT_COUNT件を保持する（超過分は遠い機体から捨てる）
 * 距離計算に必要な基準地点（lat/lng）は、内部でstorage_handler経由で読み込む
 * 
 * @param[in] rawJson AirLabs APIから取得した生のJSONレスポンス
 * @param[out] flights 機体データの格納先配列（距離の昇順ソート済）
 * @param[out] flightCount 配列に格納された機体数格納先
 * @param[out] remainingRequests レスポンスのrequest.key.limits_totalから取得した、APIキーの残りリクエスト数格納先
 * @param[out] errorOut 失敗時（JSON解析失敗、またはAirLabs APIエラー）のmessage/code格納先
 * @return true 成功（取得件数0件の場合を含む）
 * @return false 失敗（errorOut参照）
 */
bool parseFlightsResponse(const String& rawJson, FlightData flights[], int& flightCount, int& remainingRequests, ErrorData& errorOut);

/**
 * @brief NTPで日本標準時に同期する
 * 
 * 呼び出し時点でWi-Fi接続済みであることが前提
 * 
 * @param[out] timeInfo NTP同期が成功した場合の、同期結果の日時情報格納先
 * @return true 同期成功（timeInfoに結果を格納）
 * @return false 同期失敗
 */
bool syncTime(struct tm& timeInfo);

/**
 * @brief 2点間の地表距離を計算する（単位：メートル、Haversine公式）
 * 
 * TinyGPSPlusライブラリが提供する同名の静的関数を、この1関数のためだけにライブラリ全体を依存させることを避けるため、自前実装に置き換えたもの
 * ※度→ラジアン変換は、Arduino.hでマクロ定義されているradians(deg)を使用
 * 
 * @param[in] lat1 地点1の緯度
 * @param[in] lng1 地点1の経度
 * @param[in] lat2 地点2の緯度
 * @param[in] lng2 地点2の経度
 * @return 2点間の地表距離(単位:メートル)
 */
double calculateDistanceMeters(double lat1, double lng1, double lat2, double lng2);

#endif