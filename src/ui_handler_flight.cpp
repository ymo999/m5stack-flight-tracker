/**
 * ui_handler_flight.cpp
 * 機体情報表示画面（FLIGHT_VIEW）の描画処理
 */

#include "ui_handler.h"

#include <M5Unified.h>

#include "airline_dict.h"
#include "flight_data.h"
#include "system_status.h"

// 機体情報表示画面の背景色・外枠のY座標範囲
#define FLIGHT_VIEW_FRAME_Y 35
#define FLIGHT_VIEW_FRAME_HEIGHT 200

// ============================================================
// 機体情報表示画面（FLIGHT_VIEW）を描画する
// ============================================================
void drawFlightView() {
    FlightData& flight = foundFlights[currentDisplayIndex];

    M5.Lcd.fillScreen(TFT_BLACK);
    M5.Lcd.setTextColor(TFT_WHITE);
    M5.Lcd.setTextSize(2);
    M5.Lcd.setTextDatum(top_left);

    // ------------------------------------------------------
    // 取得日時（左上）
    // ------------------------------------------------------
    M5.Lcd.setCursor(5, 11);
    M5.Lcd.print("Updated : ");
    M5.Lcd.print(lastUpdateTime);

    // ------------------------------------------------------
    // 電池アイコン（右上）
    // ------------------------------------------------------
    drawBatteryIcon(288, 13, batteryLevel);

    // ------------------------------------------------------
    // 外枠（便名エリア〜情報項目エリアを囲む）
    // ------------------------------------------------------
    M5.Lcd.drawRect(5, FLIGHT_VIEW_FRAME_Y, 310, FLIGHT_VIEW_FRAME_HEIGHT, TFT_WHITE);

    // ------------------------------------------------------
    // 便名（左）＋航空会社名（右、切り詰めあり）
    // ------------------------------------------------------
    String callsignText = (flight.callsign.length() > 0) ? flight.callsign : "---";
    M5.Lcd.setCursor(12, 40);
    M5.Lcd.print(callsignText);

    String airlineName = (flight.airlineIcao.length() > 0)
        ? String(getAirlineName(flight.airlineIcao.c_str()))
        : "---";
    M5.Lcd.setCursor(92, 40);
    M5.Lcd.print(truncateText(airlineName.c_str(), 18));

    // ------------------------------------------------------
    // 便名・航空会社名エリアの下の区切り線
    // ------------------------------------------------------
    M5.Lcd.drawLine(5, 65, 315, 65, TFT_WHITE);

    // ------------------------------------------------------
    // ROUTE
    // ------------------------------------------------------
    String fromText = (flight.from.length() > 0) ? flight.from : "---";
    String toText = (flight.to.length() > 0) ? flight.to : "---";
    M5.Lcd.setCursor(12, 71);
    M5.Lcd.print("ROUTE");
    M5.Lcd.setCursor(100, 71);
    M5.Lcd.print(fromText + " -> " + toText);

    // ------------------------------------------------------
    // ALTITUDE（-1は欠損値。カンマ区切りを適用）
    // ------------------------------------------------------
    String altText = (flight.alt >= 0) ? (addThousandsSeparator(flight.alt) + " m") : "---";
    M5.Lcd.setCursor(12, 90);
    M5.Lcd.print("ALT");
    M5.Lcd.setCursor(100, 90);
    M5.Lcd.print(altText);

    // ------------------------------------------------------
    // SPEED（-1は欠損値）
    // ------------------------------------------------------
    String speedText = (flight.speed >= 0) ? (String(flight.speed) + " km/h") : "---";
    M5.Lcd.setCursor(12, 109);
    M5.Lcd.print("SPEED");
    M5.Lcd.setCursor(100, 109);
    M5.Lcd.print(speedText);

    // ------------------------------------------------------
    // HEADING（-1は欠損値。8方位表記を併記）
    // ------------------------------------------------------
    String headingText = (flight.heading >= 0)
        ? (String(flight.heading) + " (" + getDirectionLabel(flight.heading) + ")")
        : "---";
    M5.Lcd.setCursor(12, 128);
    M5.Lcd.print("HEADING");
    M5.Lcd.setCursor(100, 128);
    M5.Lcd.print(headingText);

    // ------------------------------------------------------
    // TYPE
    // ------------------------------------------------------
    String typeText = (flight.type.length() > 0) ? flight.type : "---";
    M5.Lcd.setCursor(12, 147);
    M5.Lcd.print("TYPE");
    M5.Lcd.setCursor(100, 147);
    M5.Lcd.print(typeText);

    // ------------------------------------------------------
    // DISTANCE（lat/lng欠損時はparseFlightsResponse側で除外済みのため、常に値を持つ）
    // ------------------------------------------------------
    char distBuf[16];
    sprintf(distBuf, "%.1f km", flight.dist);
    M5.Lcd.setCursor(12, 166);
    M5.Lcd.print("DIST");
    M5.Lcd.setCursor(100, 166);
    M5.Lcd.print(distBuf);

    // ------------------------------------------------------
    // SQUAWK（レスポンスに含まれる場合のみ表示、それ以外はプレースホルダ）
    // ------------------------------------------------------
    String squawkText = (flight.squawk.length() > 0) ? flight.squawk : "---";
    M5.Lcd.setCursor(12, 185);
    M5.Lcd.print("SQUAWK");
    M5.Lcd.setCursor(100, 185);
    M5.Lcd.print(squawkText);

    // ------------------------------------------------------
    // QRコード（FlightAware用URL。flightIcaoが無い場合は生成できないためプレースホルダ表示）
    // ------------------------------------------------------
    if (flight.flightIcao.length() > 0) {
        String flightAwareUrl = "https://flightaware.com/live/flight/" + flight.flightIcao;
        M5.Lcd.qrcode(flightAwareUrl.c_str(), 218, 71, 90, 2);
    } else {
        M5.Lcd.setTextDatum(middle_center);
        M5.Lcd.drawString("---", 263, 116);
        M5.Lcd.setTextDatum(top_left);
    }

    // ------------------------------------------------------
    // 通し番号（QRコード下）
    // ------------------------------------------------------
    M5.Lcd.setCursor(233, 166);
    M5.Lcd.print(currentDisplayIndex + 1);
    M5.Lcd.setCursor(260, 166);
    M5.Lcd.print("of");
    M5.Lcd.setCursor(283, 166);
    M5.Lcd.print(totalFlightCount);

    // ------------------------------------------------------
    // ボタンラベル・区切り線
    // ------------------------------------------------------
    M5.Lcd.drawLine(5, 204, 315, 204, TFT_WHITE);
    drawButtonLabels("PREV", "NEXT", "SET");

}