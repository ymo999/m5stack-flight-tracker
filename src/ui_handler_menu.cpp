/**
 * ui_handler_menu.cpp
 * 設定メニュー画面（SETTINGS）の描画処理
 */

#include "ui_handler.h"                         // M5Unified.hはこの中でインクルード済

#include "state_machine.h"

// SETTINGS画面のレイアウト用マクロ
#define SETTINGS_FRAME_Y 34
#define SETTINGS_FRAME_HEIGHT 200
#define SETTINGS_ITEM_START_Y 41
#define SETTINGS_ITEM_LINE_HEIGHT 19
#define SETTINGS_SEPARATOR_Y 136
#define SETTINGS_RESET_ALL_Y 141

// SETTINGS画面の項目一覧（RESET ALLを除く。RESET ALLは別枠で個別描画するため配列に含めない）
const char* settingsItems[] = {"LOCATION", "API KEY", "Wi-Fi", "SCAN RANGE", "SHOW CONFIG"};

// ============================================================
// 設定メニュー画面（SETTINGS）を描画する
// ============================================================
void drawSettingsView() {

    // 画面共通の初期化
    initScreenDrawing();

    // ------------------------------------------------------
    // タイトル（中央揃え）
    // ------------------------------------------------------
    M5.Lcd.setTextDatum(top_center);
    M5.Lcd.drawString("SETTINGS", 160, 8);
    M5.Lcd.setTextDatum(top_left);

    // ------------------------------------------------------
    // 外枠
    // ------------------------------------------------------
    M5.Lcd.drawRect(5, SETTINGS_FRAME_Y, 310, SETTINGS_FRAME_HEIGHT, TFT_WHITE);

    // ------------------------------------------------------
    // 通常項目（LOCATION〜SHOW CONFIG）
    // ------------------------------------------------------
    for (int i = 0; i < SETTINGS_REGULAR_ITEM_COUNT; i++) {
        int y = SETTINGS_ITEM_START_Y + i * SETTINGS_ITEM_LINE_HEIGHT;
        bool isSelected = (i == cursorIndex);

        // 選択項目のハイライト
        drawCursorHighlight(12, y, 300, SETTINGS_ITEM_LINE_HEIGHT, settingsItems[i], isSelected);
    }

    // ------------------------------------------------------
    // 区切り線（RESET ALLとの区切り）
    // ------------------------------------------------------
    M5.Lcd.drawFastHLine(5, SETTINGS_SEPARATOR_Y, 310, TFT_DARKGREY);

    // ------------------------------------------------------
    // RESET ALL（区切り線の下、赤文字を選択時も維持）
    // ------------------------------------------------------
    bool isResetSelected = (cursorIndex == SETTINGS_TOTAL_ITEM_COUNT - 1);

    // 選択項目のハイライト
    drawCursorHighlight(12, SETTINGS_RESET_ALL_Y, 300, SETTINGS_ITEM_LINE_HEIGHT, "RESET ALL", isResetSelected, TFT_RED, false);

    // ------------------------------------------------------
    // ボタンラベル・区切り線
    // ------------------------------------------------------
    drawButtonLabels("BACK", "DOWN", "SELECT");
}