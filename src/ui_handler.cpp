/**
 * ui_handler.cpp
 * UI描画関連の実装
 */

#include "ui_handler.h"                         // M5Unified.hはこの中でインクルード済

#include "flight_data.h"
#include "system_status.h"

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
// 2桁未満の数値を、先頭に"0"を付けて2桁の文字列にする（ゼロ埋め）
// sprintf不使用（vfprintf系の実装がリンクされフラッシュ容量を圧迫するため、Stringクラスの機能で代替）
// ============================================================
static String padZero2(int value) {
    return (value < 10) ? ("0" + String(value)) : String(value);
}

// ============================================================
// 時刻情報をMM/DD HH:MM形式の文字列に整形する
// ============================================================
String formatUpdateTime(const struct tm& timeInfo) {
    return padZero2(timeInfo.tm_mon + 1) + "/" + padZero2(timeInfo.tm_mday) + " "
         + padZero2(timeInfo.tm_hour) + ":" + padZero2(timeInfo.tm_min);
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
// 文字列の先頭・末尾4文字を残し、中間をマスクする（APIキー表示用）
// 8文字未満の場合、先頭・末尾が重複してしまうため全体を "****" に置き換える
// ============================================================
String maskSecret(const String& text) {
    if (text.length() < 8) {
        return "****";
    }
    return text.substring(0, 4) + "****...****" + text.substring(text.length() - 4);
}

// ============================================================
// 画面全体の初期化（背景クリア・共通テキスト設定）
// ============================================================
void initScreenDrawing() {
    M5.Lcd.fillScreen(TFT_BLACK);
    M5.Lcd.setTextColor(TFT_WHITE);
    M5.Lcd.setTextSize(2);
    M5.Lcd.setTextDatum(top_left);
    M5.Lcd.setTextWrap(false);                          // 画面右端での自動折り返しを無効化（はみ出しはそのまま見切れる）
}

// ============================================================
// タイトルを持たない画面共通の外枠を描画する
// （エラー画面のみ、messageの自動折り返しが枠線を越えるため対象外）
// ============================================================
void drawOuterFrame() {
    M5.Lcd.drawRect(5, 6, 310, 228, TFT_WHITE);
}

// ============================================================
// タイトルを持つ画面共通の外枠を描画する
// ============================================================
void drawOuterFrameWithTitle() {
    M5.Lcd.drawRect(5, 34, 310, 200, TFT_WHITE);
}

// ============================================================
// タイトルを描画する
// ============================================================
void drawTitle(const char* title) {
    M5.Lcd.setTextDatum(top_center);
    M5.Lcd.setTextSize(2);
    M5.Lcd.drawString(title, 160, 8);
    M5.Lcd.setTextDatum(top_left);                      // 左揃え描画に戻す
}

// ============================================================
// 3つのボタンラベルを描画する
// ============================================================
void drawButtonLabels(const char* labelA, const char* labelB, const char* labelC) {

    M5.Lcd.setTextColor(TFT_WHITE);                                                 // 直前の描画で色が変更されていても、ボタンラベルを常に白に固定するため
    M5.Lcd.drawFastHLine(BUTTON_AREA_MARGIN, BUTTON_AREA_Y, 310, TFT_WHITE);        // ボタンエリアの区切り線

    int y = 220;
    int margin = BUTTON_AREA_MARGIN;
    int areaWidth = BUTTON_AREA_WIDTH;

    M5.Lcd.setTextDatum(middle_center);
    if (labelA != nullptr) M5.Lcd.drawString(labelA, margin + areaWidth * 0 + areaWidth / 2, y);
    if (labelB != nullptr) M5.Lcd.drawString(labelB, margin + areaWidth * 1 + areaWidth / 2, y);
    if (labelC != nullptr) M5.Lcd.drawString(labelC, margin + areaWidth * 2 + areaWidth / 2, y);
}

// ============================================================
// カーソル選択状態を反映した項目を描画する
// 選択中は背景を白で塗りつぶす
// 文字色は forceBlackOnSelect が true の場合のみ黒に強制し、
// false の場合は指定された textColor をそのまま維持する
// （例：RESET ALLは選択時も赤文字を保ちたいため forceBlackOnSelect=false を指定）
// ============================================================
void drawCursorHighlight(int x, int y, int width, int height, const char* text, bool isSelected, 
                        uint16_t textColor, bool forceBlackOnSelect) {
    if (isSelected) {
        M5.Lcd.fillRect(x, y, width, height, TFT_WHITE);
        M5.Lcd.setTextColor(forceBlackOnSelect ? TFT_BLACK : textColor);
    } else {
        M5.Lcd.setTextColor(textColor);
    }

    M5.Lcd.setTextDatum(middle_left);
    M5.Lcd.drawString(text, x + 5, y + height / 2);   // 左端に5pxの余白を設けて左寄せ
}

// ============================================================
// 電池残量アイコンを描画する
// ============================================================
void drawBatteryIcon(int x, int y, int level) {
    // 非対応（-1）または想定外の値の場合は描画しない（安全策）
    if (level < 0 || level > 100) return;

    // 外枠と突起部分（プラス端子側）
    M5.Lcd.drawRect(x, y, 24, 12, TFT_WHITE);
    M5.Lcd.fillRect(x + 24, y + 3, 3, 6, TFT_WHITE);

    // 残量バー（閾値以下は警告色で表示）
    int fillWidth = (level * 20) / 100;
    uint16_t color = (level <= BATTERY_LOW_THRESHOLD) ? TFT_RED : TFT_WHITE;
    M5.Lcd.fillRect(x + 2, y + 2, fillWidth, 8, color);
}
