/**
 * flight_data.h
 * 航空機データ構造体・定数定義
 */

#ifndef FLIGHT_DATA_H                   // インクルードガード（二重定義防止）
#define FLIGHT_DATA_H

#include <Arduino.h>

// 保持する機体データの上限
#define MAX_FLIGHT_COUNT 10

// 航空機1機分のデータを保持する構造体
// 文字列項目...空文字列、数値項目...0の場合、当該項目を欠損値として扱う
// ただし下記数値項目については0が実測値となるケースもありうるため、-1を欠損値として扱う
// alt, speed, heading
struct FlightData {
    String callsign;                    // 便名（flight_icao。取得できない場合はflight_iataをフォールバック）
    String flightIcao;                  // 便名ICAOコード（FlightAware用URL生成に使用）
    String airlineIcao;                 // 航空会社ICAOコード（航空会社辞書の検索キー）
    double dist;                        // 基準地点からの距離（km）
    int alt;                            // 高度
    String from;                        // 出発地
    String to;                          // 到着地
    String type;                        // 機種
    int speed;                          // 巡航速度
    int heading;                        // 方位
    String squawk;                      // スコーク（レスポンスに含まれない場合は空文字）
};

// 航空機データの配列・件数・現在の表示インデックス
extern FlightData foundFlights[MAX_FLIGHT_COUNT];
extern int totalFlightCount;
extern int currentDisplayIndex;

// 直近のデータ取得日時（MM/DD HH:MM形式。未取得時は"--/-- --:--"）
extern String lastUpdateTime;

#endif