/**
 * state_machine.cpp
 * 画面状態（SystemMode）管理・状態遷移関連の実装
 */

#include "state_machine.h"

#include <M5Unified.h>

#include "flight_data.h"
#include "storage_handler.h"                // handleConfirmDialog()のloadRemainingRequests()、およびclearWifiCredentials()等（executeResetAll()での全設定消去）で使用
#include "ui_handler.h"

// 現在の画面状態の実体
// 起動時の分岐ロジック（Wi-Fi接続・APIキー・基準地点の登録状況による初期状態の判定）は未実装のため、
// 暫定的にMODE_WIFI_SETUP固定とする
SystemMode currentMode = MODE_WIFI_SETUP;

// 現在表示中の確認ダイアログの種別の実体
// MODE_CONFIRM_DIALOG以外の状態では意味を持たないため、初期値はCONFIRM_NONEとする
ConfirmTarget currentConfirm = CONFIRM_NONE;

// 画面の再描画が必要かどうかを示すフラグの実体
// 起動直後は必ず1回描画するため、初期値はtrueとする
bool needsRedraw = true;

// 現在アクティブな画面におけるカーソル位置の実体
// 画面切り替え時に0へリセットするため、初期値の意味は薄いが便宜上0とする
int cursorIndex = 0;

// プロトタイプ宣言
// state_machine.cpp内でのみ使用する関数
// （state_machine.hには公開しない。RESET ALL実行処理のため、handleConfirmDialog()より下に実装を配置）
void executeResetAll();

// ============================================================
// 状態管理機構の初期化
// ============================================================
void initStateMachine() {
    currentMode = MODE_WIFI_SETUP;
    needsRedraw = true;
    Serial.printf("[INIT] initStateMachine() done. currentMode = %d\n", currentMode);
}

// ============================================================
// 現在の画面状態に応じて、対応するハンドラ関数を呼び出す
// ============================================================
void updateStateMachine() {
    switch (currentMode) {
        case MODE_WIFI_SETUP:      handleWiFiSetupView();  break;
        case MODE_FLIGHT_VIEW:     handleFlightView();     break;
        case MODE_MENU_VIEW:       handleMenuView();       break;
        case MODE_CONFIG_VIEW:     handleConfigView();     break;
        case MODE_SCAN_RANGE_VIEW: handleScanRangeView();  break;
        case MODE_QR_VIEW:         handleQrView();         break;
        case MODE_CONFIRM_DIALOG:  handleConfirmDialog();  break;
        case MODE_ERROR_VIEW:      handleErrorView();      break;
        case MODE_NO_FLIGHTS_VIEW: handleNoFlightsView();  break;
        case MODE_LOADING:         handleLoadingView();    break;
    }
}

// ============================================================
// 各画面状態（SystemMode）に対応するハンドラ関数
// 画面の描画・ボタン処理の分岐を担当する
// ============================================================

// Wi-Fi設定関連（AP接続案内／接続成功／接続失敗）
void handleWiFiSetupView() {
    /* TODO : 処理内容の記述（Wi-Fi設定関連画面の描画・ボタン処理、5.2・5.9参照） */
}

// 機体情報表示
void handleFlightView() {
    // ------------------------------------------------------
    // ボタン処理（毎回実行）
    // ------------------------------------------------------
    if (M5.BtnA.wasPressed()) {
        // PREV：前の機体へ。1機目の場合は最終機体へループ（再取得は促さない）
        currentDisplayIndex--;
        if (currentDisplayIndex < 0) {
            currentDisplayIndex = totalFlightCount - 1;
        }
        needsRedraw = true;
        Serial.printf("[BTN] BtnA wasPressed. currentDisplayIndex = %d\n", currentDisplayIndex);
        return;
    }

    if (M5.BtnB.wasPressed()) {
        // NEXT：次の機体へ。最終機体の場合は再取得の確認ダイアログへ遷移
        if (currentDisplayIndex >= totalFlightCount - 1) {
            currentMode = MODE_CONFIRM_DIALOG;
            currentConfirm = CONFIRM_REFRESH;
            Serial.printf("[BTN] BtnB wasPressed. currentMode = %d, currentConfirm = %d\n", currentMode, currentConfirm);
        } else {
            currentDisplayIndex++;
            Serial.printf("[BTN] BtnB wasPressed. currentDisplayIndex = %d\n", currentDisplayIndex);
        }
        needsRedraw = true;
        return;
    }

    if (M5.BtnC.wasPressed()) {
        // SET：設定メニュー画面へ
        currentMode = MODE_MENU_VIEW;
        cursorIndex = 0;
        needsRedraw = true;
        Serial.printf("[BTN] BtnC wasPressed. currentMode = %d\n", currentMode);
        return;
    }

    // ------------------------------------------------------
    // 描画処理（needsRedrawがtrueの時のみ実行）
    // ------------------------------------------------------
    if (needsRedraw) {
        drawFlightView();
        needsRedraw = false;
        Serial.println("[VIEW] FLIGHT VIEW redraw");
    }
}

// 設定メニュー（SETTINGS）
void handleMenuView() {
    // ------------------------------------------------------
    // ボタン処理（毎回実行）
    // ------------------------------------------------------
    if (M5.BtnA.wasPressed()) {
        // BACK：機体情報表示画面へ戻る
        currentMode = MODE_FLIGHT_VIEW;
        needsRedraw = true;
        Serial.printf("[BTN] BtnA wasPressed. currentMode = %d\n", currentMode);
        return;
    }

    if (M5.BtnB.wasPressed()) {
        // DOWN：カーソルを1つ下へ。最後（RESET ALL）の次は先頭（LOCATION）へループ
        cursorIndex++;
        if (cursorIndex >= SETTINGS_TOTAL_ITEM_COUNT) {
            cursorIndex = 0;
        }
        needsRedraw = true;
        Serial.printf("[BTN] BtnB wasPressed. cursorIndex = %d\n", cursorIndex);
        return;
    }

    if (M5.BtnC.wasPressed()) {
        // SELECT：選択中の項目に応じた画面遷移
        // TODO：LOCATION・API KEY・SCAN RANGE・RESET ALL（手順21・22・25等）の分岐は未実装
        // TODO：可読性の観点では、例えばenumや#defineで項目のインデックスに名前を付けた方が良い
        if (cursorIndex == 2) {                 // Wi-Fi
            currentMode = MODE_CONFIRM_DIALOG;
	        currentConfirm = CONFIRM_WIFI_SETTINGS;
            needsRedraw = true;
        }
        if (cursorIndex == 4) {                 // SHOW CONFIG
            currentMode = MODE_CONFIG_VIEW;
            needsRedraw = true;
        }
        if (cursorIndex == 5) {                 // RESET ALL
            currentMode = MODE_CONFIRM_DIALOG;
	        currentConfirm = CONFIRM_RESET;
            needsRedraw = true;
        }
        Serial.printf("[BTN] BtnC wasPressed. currentMode = %d, currentConfirm = %d\n", currentMode, currentConfirm);
        return;
    }

    // ------------------------------------------------------
    // 描画処理（needsRedrawがtrueの時のみ実行）
    // ------------------------------------------------------
    if (needsRedraw) {
        drawSettingsView();
        needsRedraw = false;
        Serial.println("[VIEW] SETTINGS redraw");
    }
}

// 設定内容一覧（CONFIG）
void handleConfigView() {
    // ------------------------------------------------------
    // ボタン処理（毎回実行）
    // ------------------------------------------------------
    if (M5.BtnA.wasPressed()) {
        // BACK：設定メニュー画面へ戻る
        currentMode = MODE_MENU_VIEW;
        needsRedraw = true;
        Serial.printf("[BTN] BtnA wasPressed. currentMode = %d\n", currentMode);
        return;
    }

    // ------------------------------------------------------
    // 描画処理（needsRedrawがtrueの時のみ実行）
    // ------------------------------------------------------
    if (needsRedraw) {
        drawConfigView();
        needsRedraw = false;
        Serial.println("[VIEW] CONFIG redraw");
    }
}

// SCAN RANGE選択
void handleScanRangeView() {
    /* TODO : 処理内容の記述（SCAN RANGE選択画面の描画・ボタン処理、5.2・5.7.3参照） */
}

// QRコード誘導（APIキー／基準地点）
void handleQrView() {
    /* TODO : 処理内容の記述（QRコード誘導画面の描画・ボタン処理、5.2・5.8参照） */
}

// 確認ダイアログ
void handleConfirmDialog() {
    if (M5.BtnA.wasPressed()) {
        // CANCEL：currentConfirmに応じて戻り先を分岐
        switch (currentConfirm) {
            case CONFIRM_RESET:
            case CONFIRM_WIFI_SETTINGS:
                currentMode = MODE_MENU_VIEW;
                break;
            case CONFIRM_WIFI_RECONNECT:
            case CONFIRM_REFRESH:
                currentMode = MODE_FLIGHT_VIEW;
                currentDisplayIndex = 0;                // 1機目に戻る
                break;
            default:
                break;
        }
        currentConfirm = CONFIRM_NONE;
        needsRedraw = true;
        Serial.printf("[BTN] BtnA wasPressed. currentMode = %d\n", currentMode);
        return;
    }

    if (M5.BtnC.wasPressed()) {
        // CONFIRM：currentConfirmに応じて実行処理を分岐
        switch (currentConfirm) {
            case CONFIRM_RESET:
                executeResetAll();                      // 内部でESP.restart()するため以降は戻らない
                break;
            case CONFIRM_WIFI_SETTINGS:
            case CONFIRM_WIFI_RECONNECT:
                /* TODO : Wi-Fi再設定処理（手順25で実装） */
                break;
            case CONFIRM_REFRESH:
                /* TODO : データ再取得処理 */
                break;
            default:
                break;
        }
        currentConfirm = CONFIRM_NONE;
        needsRedraw = true;
        Serial.printf("[BTN] BtnC wasPressed. currentMode = %d\n", currentMode);
        return;
    }

    // ------------------------------------------------------
    // 描画処理（needsRedrawがtrueの時のみ実行）
    // currentConfirmに応じて表示文言を出し分ける
    // ------------------------------------------------------
    if (needsRedraw) {
        switch (currentConfirm) {
            case CONFIRM_RESET:
                drawConfirmDialog("Reset all settings?",
                                  "This will erase Wi-Fi, API key,",
                                  "location, and cached data.");
                break;

            case CONFIRM_WIFI_SETTINGS:
            case CONFIRM_WIFI_RECONNECT:
                drawConfirmDialog("Change Wi-Fi settings?",
                                  "The current connection",
                                  "will be disconnected.");
                break;

            case CONFIRM_REFRESH: {
                int remainingRequests = 0;
                loadRemainingRequests(remainingRequests);

                char remainBuf[24];
                sprintf(remainBuf, "%d requests left.", remainingRequests);

                drawConfirmDialog("Refresh flight data?",
                                  "This will use one API request.",
                                  remainBuf);
                break;
            }

            default:
                break;
        }
        needsRedraw = false;
        Serial.printf("[VIEW] CONFIRM DIALOG redraw. currentConfirm = %d\n", currentConfirm);
    }
}

// エラー表示
void handleErrorView() {
    /* TODO : 処理内容の記述（エラー画面の描画・ボタン処理、5.2・5.10参照） */
}

// 機体0件
void handleNoFlightsView() {
    /* TODO : 処理内容の記述（機体0件画面の描画・ボタン処理、5.2・5.10参照） */
}

// データ取得中
void handleLoadingView() {
    /* TODO : 処理内容の記述（ローディング画面の描画、5.2・5.11参照） */
}

// ============================================================
// 全設定を消去し、実機を再起動する（RESET ALL実行処理）
// 消去処理を全て完了させてからESP.restart()を呼ぶこと（順序を入れ替えないこと）
// ※ESP.restart()はハードウェアレベルの再起動のため、グローバル変数・静的変数を
//   確実に初期値へ戻せる。ソフトウェア的な再初期化（initStateMachine()等）は、
//   将来変数が増えるたびに初期化漏れのリスクを抱えるため採用しない
// ============================================================

void executeResetAll() {
    // Wi-Fi資格情報（NVS "wifi"名前空間）を消去
    clearWifiCredentials();

    // APIキー（NVS "api"名前空間）を消去
    clearApiKey();

    // 取得地点・SCAN RANGE・静的IP設定（config.json）を消去
    clearConfig();

    // 機体情報キャッシュ・残りリクエスト数（cache.json）を消去
    clearCache();

    Serial.println("[RESET] All settings cleared. Restarting...");

    // 実機再起動（全グローバル変数・静的変数が確実に初期状態へ戻る）
    ESP.restart();
}