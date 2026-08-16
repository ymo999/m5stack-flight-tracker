/**
 * ui_handler_scanrange.cpp
 * SCAN RANGE選択画面の描画処理
 */

#include "ui_handler.h"                         // M5Unified.hはこの中でインクルード済

#include "state_machine.h"
#include "storage_handler.h"                    // loadConfig()で現在の設定値を取得するため

// SCAN RANGE画面のレイアウト用マクロ
#define SCAN_RANGE_FRAME_Y 34
#define SCAN_RANGE_FRAME_HEIGHT 200
#define SCAN_RANGE_ITEM_START_Y 41
#define SCAN_RANGE_ITEM_LINE_HEIGHT 19

// SCAN RANGE画面の項目一覧（config.jsonのscanRange値と直接対応する文字列）
const char* scanRangeItems[] = {"NARROW", "WIDE"};

// ============================================================
// SCAN RANGE選択画面を描画する
// ============================================================
void drawScanRangeView() {

    // 画面共通の初期化
    initScreenDrawing();

    // ------------------------------------------------------
    // タイトル（中央揃え）
    // ------------------------------------------------------
    M5.Lcd.setTextDatum(top_center);
    M5.Lcd.drawString("SCAN RANGE", 160, 8);
    M5.Lcd.setTextDatum(top_left);

    // ------------------------------------------------------
    // 外枠
    // ------------------------------------------------------
    M5.Lcd.drawRect(5, SCAN_RANGE_FRAME_Y, 310, SCAN_RANGE_FRAME_HEIGHT, TFT_WHITE);

    // ------------------------------------------------------
    // 現在の設定値を取得（黄色文字で強調表示するため）
    // ------------------------------------------------------
    ConfigData config;
    loadConfig(config);

    // ------------------------------------------------------
    // 項目一覧（NARROW・WIDE）
    // ------------------------------------------------------
    for (int i = 0; i < SCAN_RANGE_ITEM_COUNT; i++) {
        int y = SCAN_RANGE_ITEM_START_Y + i * SCAN_RANGE_ITEM_LINE_HEIGHT;
        bool isSelected = (i == scanRangeCursorIndex);
        bool isCurrentValue = (config.scanRange == scanRangeItems[i]);

        if (isCurrentValue) {
            // 現在の設定値：黄色文字を維持（カーソルが重なっても黒文字に変えない）
            drawCursorHighlight(12, y, 300, SCAN_RANGE_ITEM_LINE_HEIGHT, scanRangeItems[i], isSelected, TFT_YELLOW, false);
        } else {
            // 通常項目：デフォルト（白文字、選択時は黒文字）
            drawCursorHighlight(12, y, 300, SCAN_RANGE_ITEM_LINE_HEIGHT, scanRangeItems[i], isSelected);
        }
    }

    // ------------------------------------------------------
    // ボタンラベル・区切り線
    // ------------------------------------------------------
    drawButtonLabels("BACK", "DOWN", "SELECT");
}