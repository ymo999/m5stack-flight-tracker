/**
 * ui_handler.cpp
 * UI描画関連の実装
 */

#include "ui_handler.h"

#include <M5Unified.h>

#include "flight_data.h"

// ============================================================
// 進行方向（度数）を8方位の英字表記に変換する
// 各方位を「中心角 ± 22.5°」の範囲に分ける
// （例）Nは337.5°〜22.5°の範囲、NEは22.5°〜67.5°の範囲
// ============================================================
const char* getDirectionLabel(float heading) {
    int index = (int)((heading + 22.5) / 45.0) % 8;
    const char* directions[] = {"N", "NE", "E", "SE", "S", "SW", "W", "NW"};
    return directions[index];
}

// ============================================================
// 時刻情報をMM/DD HH:MM形式の文字列に整形する
// ============================================================
String formatUpdateTime(const struct tm& timeInfo) {
    char buf[12];                       // 表示文字+終端文字\0
    sprintf(buf, "%02d/%02d %02d:%02d",
            timeInfo.tm_mon + 1,        // tm_monは0始まりのため+1
            timeInfo.tm_mday,
            timeInfo.tm_hour,
            timeInfo.tm_min);
    return String(buf);
}

// ============================================================
// 指定文字数に収まらない場合、末尾3文字分を "..." に置き換える
// ============================================================
String truncateText(const char* text, int maxLen) {
    String s = String(text);
    if (s.length() <= maxLen) {
        return s;
    }
    return s.substring(0, maxLen - 3) + "...";
}

// ============================================================
// 整数値を3桁ごとにカンマ区切りした文字列に変換する
// ============================================================
String addThousandsSeparator(int value) {
    bool isNegative = (value < 0);
    String numStr = String(isNegative ? -value : value);
    String result = "";

    int len = numStr.length();
    for (int i = 0; i < len; i++) {
        if (i > 0 && (len - i) % 3 == 0) {
            result += ",";
        }
        result += numStr[i];
    }

    return isNegative ? ("-" + result) : result;
}

// ============================================================
// 3つのボタンラベルを描画する
// ============================================================
void drawButtonLabels(const char* labelA, const char* labelB, const char* labelC) {
    int y = 220;
    int margin = 5;
    int usableWidth = 320 - (margin * 2);   // 310px
    int areaWidth = usableWidth / 3;

    M5.Lcd.setTextDatum(middle_center);
    if (labelA != nullptr) M5.Lcd.drawString(labelA, margin + areaWidth * 0 + areaWidth / 2, y);
    if (labelB != nullptr) M5.Lcd.drawString(labelB, margin + areaWidth * 1 + areaWidth / 2, y);
    if (labelC != nullptr) M5.Lcd.drawString(labelC, margin + areaWidth * 2 + areaWidth / 2, y);
}