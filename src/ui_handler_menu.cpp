/**
 * ui_handler_menu.cpp
 * 設定メニュー画面（SETTINGS）の描画処理
 */

#include "ui_handler.h"                         // M5Unified.hはこの中でインクルード済

#include "state_machine.h"

// SETTINGS画面のレイアウト用マクロ
#define SETTINGS_ITEM_START_Y 41
#define SETTINGS_ITEM_LINE_HEIGHT 19
#define SETTINGS_SEPARATOR_Y 136
#define SETTINGS_RESET_ALL_Y 141

// SETTINGS画面の項目一覧（RESET ALLを含む全項目。この配列がSETTINGS画面の項目構成の単一の情報源となる）
// enum値と表示文字列を対で保持することで、並び順が変わってもずれが生じないようにする
// ※項目数は、state_machine.hで定義しているSETTINGS_ITEM_COUNTマクロ
// 制約：RESET ALLは常に配列の末尾に置くこと
// （区切り線を挟んだ個別描画のY座標計算が「配列の最後の1件である」ことを前提にしているため）
const SettingsItemEntry settingsItems[] = {
    { SETTINGS_ITEM_LOCATION,    "LOCATION" },
    { SETTINGS_ITEM_API_KEY,     "API KEY" },
    { SETTINGS_ITEM_WIFI,        "Wi-Fi" },
    { SETTINGS_ITEM_SCAN_RANGE,  "SCAN RANGE" },
    { SETTINGS_ITEM_SHOW_CONFIG, "SHOW CONFIG" },
    { SETTINGS_ITEM_RESET_ALL,   "RESET ALL" },
};

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
    drawOuterFrameWithTitle();

    // ------------------------------------------------------
    // 項目
    // ------------------------------------------------------

    for (int i = 0; i < SETTINGS_ITEM_COUNT; i++) {
        bool isResetAll = (settingsItems[i].index == SETTINGS_ITEM_RESET_ALL);
        bool isSelected = (settingsItems[i].index == cursorIndex);

        // RESET ALL：区切り線を挟んだ下に、常に赤文字で描画。区切り線と項目の描画位置Y座標はマクロで固定
        // それ以外：先頭行（Y座標はマクロで固定）から等間隔で描画
        if (isResetAll) {
            
            // 区切り線
            M5.Lcd.drawFastHLine(5, SETTINGS_SEPARATOR_Y, 310, TFT_DARKGREY);

            // 項目の描画（選択されている場合はハイライト）
            drawCursorHighlight(12, SETTINGS_RESET_ALL_Y, 300, SETTINGS_ITEM_LINE_HEIGHT,
                                settingsItems[i].label, isSelected, TFT_RED, false);
        } else {

            // 描画位置のY座標を決定
            int y = SETTINGS_ITEM_START_Y + i * SETTINGS_ITEM_LINE_HEIGHT;

            // 項目の描画（選択されている場合はハイライト）
            drawCursorHighlight(12, y, 300, SETTINGS_ITEM_LINE_HEIGHT, settingsItems[i].label, isSelected);
        }
    }

    // ------------------------------------------------------
    // ボタンラベル・区切り線
    // ------------------------------------------------------
    drawButtonLabels("BACK", "DOWN", "SELECT");
}