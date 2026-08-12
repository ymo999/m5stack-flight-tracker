/**
 * ui_handler.cpp
 * UI描画関連の実装
 */

#include "ui_handler.h"

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