/**
 * state_machine.cpp
 * 画面状態（SystemMode）管理・状態遷移関連の実装
 */

#include "state_machine.h"

#include <M5Unified.h>

#include "api_handler.h"                    // データ取得中（MODE_LOADING）でのWi-Fi接続後の各種APIリクエスト
#include "error_data.h"
#include "flight_data.h"
#include "input_handler.h"
#include "storage_handler.h"                // 現在の設定値（APIリクエスト残数など）取得や、設定値消去を行うため
#include "ui_handler.h"
#include "wifi_handler.h"                   // データ取得中（MODE_LOADING）でのWi-Fi接続・切断

// 現在の画面状態の実体
// 実際の初期状態はinitStateMachine()がWi-Fiの状態に応じて決定するため、
// この初期値は起動シーケンス完了までの暫定値にすぎない
SystemMode currentMode = MODE_WIFI_SETUP;

// 現在表示中の確認ダイアログの種別の実体
// MODE_CONFIRM_DIALOG以外の状態では意味を持たないため、初期値はCONFIRM_NONEとする
ConfirmTarget currentConfirm = CONFIRM_NONE;

// SETTINGS画面の遷移元の実体
// MODE_MENU_VIEW以外の状態では意味を持たないため、初期値はMENUCALLER_NONEとする
MenuCaller menuCaller = MENUCALLER_NONE;

// Wi-Fi設定関連画面の表示フェーズの実体
// MODE_WIFI_SETUP以外の状態では意味を持たないため、初期値はWIFI_PHASE_NONEとする
WiFiSetupPhase wifiSetupPhase = WIFI_PHASE_NONE;

// AP接続案内画面の遷移元の実体
// MODE_WIFI_SETUP以外の状態では意味を持たないため、初期値はWIFI_CALLER_NONEとする
WiFiSetupCaller wifiSetupCaller = WIFI_CALLER_NONE;

// 画面の再描画が必要かどうかを示すフラグの実体
// 起動直後は必ず1回描画するため、初期値はtrueとする
bool needsRedraw = true;

// 現在アクティブな画面におけるカーソル位置の実体
// 画面切り替え時に0へリセットするが便宜上0で初期化
int cursorIndex = 0;

// SCAN RANGE画面専用のカーソル位置の実体
// enterScanRangeCursor()で、現在の設定値の位置に初期化される
int scanRangeCursorIndex = 0;   

// プロトタイプ宣言
// state_machine.cpp内でのみ使用する関数
// （state_machine.hには公開しない。）
void resetScanRangeCursor();                // SCAN RANGE画面のカーソル初期化処理のため、handleMenuView()より下に実装を配置
void executeResetAll();                     // RESET ALL実行処理のため、handleConfirmDialog()より下に実装を配置

// ============================================================
// 状態管理機構の初期化
// ============================================================
void initStateMachine() {
    if (isApModeActive()) {
        // 資格情報なし：initWiFi()内で既にアクセスポイント起動済→登録案内画面表示
        currentMode = MODE_WIFI_SETUP;
        wifiSetupPhase = WIFI_PHASE_GUIDE;
        wifiSetupCaller = WIFI_CALLER_INIT;
    } else if (isWiFiConnected()) {
        // 資格情報あり・接続成功：currentModeはここでは変更しない
        // TODO : APIキー・基準地点の登録状況による分岐実装
        // （実装までは呼び出し元のmain.cppの一時テストコードでcurrentModeを設定）
        Serial.println("[INIT] Wi-Fi connected. currentMode will be set by the caller (temporary)");
    } else {
        // 資格情報はあるが接続失敗
        currentMode = MODE_WIFI_SETUP;
        wifiSetupPhase = WIFI_PHASE_FAILED;
    }
    
    needsRedraw = true;
    Serial.printf("[INIT] initStateMachine() done. currentMode = %d\n", currentMode);
}

// ============================================================
// 現在の画面状態に対応する関数を呼び出す
// ============================================================
void updateStateMachine() {
    switch (currentMode) {
        case MODE_WIFI_SETUP:           handleWiFiSetupView();  break;
        case MODE_FLIGHT_VIEW:          handleFlightView();     break;
        case MODE_MENU_VIEW:            handleMenuView();       break;
        case MODE_CONFIG_VIEW:          handleConfigView();     break;
        case MODE_SCAN_RANGE_VIEW:      handleScanRangeView();  break;
        case MODE_QR_VIEW:              handleQrView();         break;
        case MODE_CONFIRM_DIALOG:       handleConfirmDialog();  break;
        case MODE_ERROR_VIEW:           handleErrorView();      break;
        case MODE_NO_FLIGHTS_VIEW:      handleNoFlightsView();  break;
        case MODE_LOADING:              handleLoadingView();    break;
        case MODE_CONNECTION_FAILED:    handleConnectionFailedView(); break;
    }
}

// ============================================================
// 各画面状態（SystemMode）に対応する処理を行う関数
// 画面の描画・ボタン処理の分岐を担当する
// ============================================================

// Wi-Fi設定関連（AP接続案内／接続失敗）
void handleWiFiSetupView() {
    // ------------------------------------------------------
    // ボタン処理（毎回実行）
    // 同じボタンでも表示フェーズによって意味が変わるため、まずフェーズで分岐する
    // ------------------------------------------------------
    if (wifiSetupPhase == WIFI_PHASE_GUIDE) {

        // BACK：設定を中断して遷移元へ戻る
        // 初回起動時は戻り先がなくラベルも非表示のため、誤操作をスルーさせる
        if (btnAWasPressed() && wifiSetupCaller != WIFI_CALLER_INIT) {
            exitAPMode();                       // 中断するためAPモード・Webサーバーを終了する

            // cursorIndexは意図的にリセットしない（SETTINGSへ戻った際に、Wi-Fi項目を選択していた位置を維持するため）
            currentMode = (wifiSetupCaller == WIFI_CALLER_SETTINGS) ? MODE_MENU_VIEW : MODE_FLIGHT_VIEW;
            wifiSetupPhase = WIFI_PHASE_NONE;
            wifiSetupCaller = WIFI_CALLER_NONE;
            needsRedraw = true;
            Serial.printf("[BTN] BtnA wasPressed. currentMode = %d\n", currentMode);
            return;
        }

    } else if (wifiSetupPhase == WIFI_PHASE_FAILED) {

        if (btnAWasPressed()) {
            // BACK：キャッシュがあれば機体情報表示へ戻り、なければ設定をやり直す
            if (totalFlightCount > 0) {
                currentMode = MODE_FLIGHT_VIEW;
                wifiSetupPhase = WIFI_PHASE_NONE;
            } else {
                // 資格情報は消さずにAPモードへ入り直す（一時的な接続失敗の可能性があるため）
                enterAPMode();
                wifiSetupPhase = WIFI_PHASE_GUIDE;
                wifiSetupCaller = WIFI_CALLER_INIT;
            }
            needsRedraw = true;
            Serial.printf("[BTN] BtnA wasPressed. currentMode = %d\n", currentMode);
            return;
        }

        if (btnCWasPressed() && totalFlightCount > 0) {
            // Wi-Fi：設定をやり直す。戻り先の機体情報表示が存在するためRECONNECT扱いとする
            // （再起動で失われた遷移元の文脈を、キャッシュの有無から補う）
            enterAPMode();
            wifiSetupPhase = WIFI_PHASE_GUIDE;
            wifiSetupCaller = WIFI_CALLER_RECONNECT;
            needsRedraw = true;
            Serial.printf("[BTN] BtnC wasPressed. currentMode = %d\n", currentMode);
            return;
        }
    }

    // ------------------------------------------------------
    // 描画処理（needsRedrawがtrueの時のみ実行）
    // ------------------------------------------------------
    if (needsRedraw) {
        if (wifiSetupPhase == WIFI_PHASE_GUIDE) {
            drawWiFiSetupGuide();
        } else {
            drawWiFiSetupFailed();
        }
        needsRedraw = false;
    }
}

// 機体情報表示
void handleFlightView() {
    // ------------------------------------------------------
    // ボタン処理（毎回実行）
    // ------------------------------------------------------
    if (btnAWasPressed()) {
        // PREV：前の機体へ。1機目の場合は最終機体へループ（再取得は促さない）
        currentDisplayIndex--;
        if (currentDisplayIndex < 0) {
            currentDisplayIndex = totalFlightCount - 1;
        }
        needsRedraw = true;
        // Serial.printf("[BTN] BtnA wasPressed. currentDisplayIndex = %d\n", currentDisplayIndex);
        return;
    }

    if (btnBWasPressed()) {
        // NEXT：次の機体へ。最終機体の場合は再取得の確認ダイアログへ遷移
        if (currentDisplayIndex >= totalFlightCount - 1) {
            currentMode = MODE_CONFIRM_DIALOG;
            currentConfirm = CONFIRM_REFRESH;
            // Serial.printf("[BTN] BtnB wasPressed. currentMode = %d, currentConfirm = %d\n", currentMode, currentConfirm);
        } else {
            currentDisplayIndex++;
            // Serial.printf("[BTN] BtnB wasPressed. currentDisplayIndex = %d\n", currentDisplayIndex);
        }
        needsRedraw = true;
        return;
    }

    if (btnCWasPressed()) {
    // SET：設定メニュー画面へ（通常の遷移元として記録）
        menuCaller = MENUCALLER_FLIGHT;
        currentMode = MODE_MENU_VIEW;
        cursorIndex = 0;
        needsRedraw = true;
        // Serial.printf("[BTN] BtnC wasPressed. currentMode = %d\n", currentMode);
        return;
    }

    // ------------------------------------------------------
    // 描画処理（needsRedrawがtrueの時のみ実行）
    // ------------------------------------------------------
    if (needsRedraw) {
        drawFlightView();
        needsRedraw = false;
        // Serial.println("[VIEW] FLIGHT VIEW redraw");
    }
}

// 設定メニュー（SETTINGS）
void handleMenuView() {
    // ------------------------------------------------------
    // ボタン処理（毎回実行）
    // ------------------------------------------------------
    // // デバッグ用
    // int targetOrCaller = 0;

    if (btnAWasPressed()) {
        // BACK：遷移元に応じて戻り先を分岐
        switch (menuCaller) {
            case MENUCALLER_NOFLIGHT:
                currentMode = MODE_NO_FLIGHTS_VIEW;
                break;
            case MENUCALLER_ERROR:
                currentMode = MODE_ERROR_VIEW;
                break;
            default:
                currentMode = MODE_FLIGHT_VIEW;
                break;
        }
        needsRedraw = true;
        // Serial.printf("[BTN] BtnA wasPressed. currentMode = %d\n", currentMode);
        return;
    }

    if (btnBWasPressed()) {
        // DOWN：カーソルを1つ下へ。最後（RESET ALL）の次は先頭（LOCATION）へループ
        cursorIndex++;
        if (cursorIndex >= SETTINGS_ITEM_COUNT) {
            cursorIndex = 0;
        }
        needsRedraw = true;
        // Serial.printf("[BTN] BtnB wasPressed. cursorIndex = %d\n", cursorIndex);
        return;
    }

    if (btnCWasPressed()) {        
        // SELECT：選択中の項目に応じた画面遷移
        switch ((SettingsItemIndex)cursorIndex) {
            case SETTINGS_ITEM_LOCATION:
                /* TODO : 基準地点設定への遷移（手順25以降で実装） */
                break;
            case SETTINGS_ITEM_API_KEY:
                /* TODO : APIキー設定への遷移（手順25以降で実装） */
                break;
            case SETTINGS_ITEM_WIFI:
                currentMode = MODE_CONFIRM_DIALOG;
                currentConfirm = CONFIRM_WIFI_SETTINGS;
                // targetOrCaller = currentConfirm;
                break;
            case SETTINGS_ITEM_SCAN_RANGE:
                resetScanRangeCursor();
                currentMode = MODE_SCAN_RANGE_VIEW;
                break;
            case SETTINGS_ITEM_SHOW_CONFIG:
                currentMode = MODE_CONFIG_VIEW;
                break;
            case SETTINGS_ITEM_RESET_ALL:
                currentMode = MODE_CONFIRM_DIALOG;
                currentConfirm = CONFIRM_RESET;
                // targetOrCaller = currentConfirm;
                break;
        }
        needsRedraw = true;
        // Serial.printf("[BTN] BtnC wasPressed. currentMode = %d, targetOrCaller = %d\n", currentMode, targetOrCaller);
        return;
    }

    // ------------------------------------------------------
    // 描画処理（needsRedrawがtrueの時のみ実行）
    // ------------------------------------------------------
    if (needsRedraw) {
        drawSettingsView();
        needsRedraw = false;
        // Serial.println("[VIEW] SETTINGS redraw");
    }
}

// 設定内容一覧（CONFIG）
void handleConfigView() {
    // ------------------------------------------------------
    // ボタン処理（毎回実行）
    // ------------------------------------------------------
    if (btnAWasPressed()) {
        // BACK：設定メニュー画面へ戻る
        currentMode = MODE_MENU_VIEW;
        needsRedraw = true;
        // Serial.printf("[BTN] BtnA wasPressed. currentMode = %d\n", currentMode);
        return;
    }

    // ------------------------------------------------------
    // 描画処理（needsRedrawがtrueの時のみ実行）
    // ------------------------------------------------------
    if (needsRedraw) {
        drawConfigView();
        needsRedraw = false;
        // Serial.println("[VIEW] CONFIG redraw");
    }
}

// SCAN RANGE選択
void handleScanRangeView() {
    // ------------------------------------------------------
    // ボタン処理（毎回実行）
    // ------------------------------------------------------
    if (btnAWasPressed()) {
        // BACK：設定メニュー画面へ戻る（範囲は変更しない）
        currentMode = MODE_MENU_VIEW;
        needsRedraw = true;
        // Serial.printf("[BTN] BtnA wasPressed. currentMode = %d, Scan range was not changed.\n", currentMode);
        return;
    }

    if (btnBWasPressed()) {
        // DOWN：カーソルを1つ下へ。最後（WIDE）の次は先頭（NARROW）へループ
        scanRangeCursorIndex++;
        if (scanRangeCursorIndex >= SCAN_RANGE_ITEM_COUNT) {
            scanRangeCursorIndex = 0;
        }
        needsRedraw = true;
        // Serial.printf("[BTN] BtnB wasPressed. scanRangeCursorIndex = %d\n", scanRangeCursorIndex);
        return;
    }

    if (btnCWasPressed()) {
        // SELECT：選択した範囲を保存し、設定メニュー画面へ戻る
        ConfigData config;
        loadConfig(config);
        config.scanRange = (scanRangeCursorIndex == 1) ? "WIDE" : "NARROW";
        saveConfig(config);

        currentMode = MODE_MENU_VIEW;
        needsRedraw = true;
        // Serial.printf("[BTN] BtnC wasPressed. currentMode = %d, currentScanRange = %s\n", currentMode, config.scanRange.c_str());
        return;
    }

    // ------------------------------------------------------
    // 描画処理（needsRedrawがtrueの時のみ実行）
    // ------------------------------------------------------
    if (needsRedraw) {
        drawScanRangeView();
        needsRedraw = false;
        // Serial.println("[VIEW] SCAN RANGE redraw");
    }
}

// QRコード誘導（APIキー／基準地点）
void handleQrView() {
    /* TODO : 処理内容の記述（QRコード誘導画面の描画・ボタン処理、5.2・5.8参照） */
}

// 確認ダイアログ
void handleConfirmDialog() {
    // ------------------------------------------------------
    // ボタン処理（毎回実行）
    // ------------------------------------------------------
    if (btnAWasPressed()) {
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
        // Serial.printf("[BTN] BtnA wasPressed. currentMode = %d\n", currentMode);
        return;
    }

    if (btnCWasPressed()) {
        // CONFIRM：currentConfirmに応じて実行処理を分岐
        switch (currentConfirm) {
            case CONFIRM_RESET:
                executeResetAll();                      // 内部でESP.restart()するため以降は戻らない
                break;

            case CONFIRM_WIFI_SETTINGS:
                // CONFIRM：資格情報・ネットワーク設定を消去してAPモードへ移行する
                // （利用者が明示的に別ネットワークへの切り替えを選んだケースのため消去してよい）
                resetAndEnterAPMode();
                currentMode = MODE_WIFI_SETUP;
                wifiSetupPhase = WIFI_PHASE_GUIDE;
                wifiSetupCaller = WIFI_CALLER_SETTINGS;
                break;

            case CONFIRM_WIFI_RECONNECT:
                // CONFIRM：上記と同じ処理だが、BACKの戻り先が異なるためcallerを分ける
                resetAndEnterAPMode();
                currentMode = MODE_WIFI_SETUP;
                wifiSetupPhase = WIFI_PHASE_GUIDE;
                wifiSetupCaller = WIFI_CALLER_RECONNECT;
                break;

            case CONFIRM_REFRESH:
                // CONFIRM：ローディング画面へ遷移する（実際の取得処理はhandleLoadingView()側で行う）
                currentMode = MODE_LOADING;
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

                // sprintf不使用（vfprintf系の実装がリンクされフラッシュ容量を圧迫するため、Stringクラスの機能で代替）
                String remainMsg = String(remainingRequests) + " requests left.";

                drawConfirmDialog("Refresh flight data?",
                                  "This will use one API request.",
                                  remainMsg.c_str());
                break;  
            }

            default:
                break;
        }
        needsRedraw = false;
        // Serial.printf("[VIEW] CONFIRM DIALOG redraw. currentConfirm = %d\n", currentConfirm);
    }
}

// エラー表示
void handleErrorView() {
    // ------------------------------------------------------
    // ボタン処理（毎回実行）
    // 機体数を判定条件に入れることにより誤操作でラベル非表示のボタンが押下されてもスルーさせる
    // ------------------------------------------------------
    if (btnAWasPressed()) {
        // BACK：キャッシュありの場合のみ表示。機体情報表示画面へ戻る
        if (totalFlightCount > 0) {
            currentMode = MODE_FLIGHT_VIEW;
            needsRedraw = true;
        }
        // Serial.printf("[BTN] BtnA wasPressed. currentMode = %d\n", currentMode);
        return;
    }

    if (btnBWasPressed()) {
        // RETRY：キャッシュなしの場合のみ表示。確認ダイアログを挟まず、そのまま再取得へ
        if (totalFlightCount == 0) {
            currentMode = MODE_LOADING;
            needsRedraw = true;
        }
        // Serial.printf("[BTN] BtnB wasPressed. currentMode = %d\n", currentMode);
        return;
    }

    if (btnCWasPressed()) {
        // SET：キャッシュなしの場合のみ表示。設定メニュー画面へ（エラー画面経由として記録）
        if (totalFlightCount == 0) {
            menuCaller = MENUCALLER_ERROR;
            currentMode = MODE_MENU_VIEW;
            cursorIndex = 0;
            needsRedraw = true;
        }
        // Serial.printf("[BTN] BtnC wasPressed. currentMode = %d\n", currentMode);
        return;
    }

    // ------------------------------------------------------
    // 描画処理（needsRedrawがtrueの時のみ実行）
    // ------------------------------------------------------
    if (needsRedraw) {
        drawErrorView(currentError.message.c_str(), currentError.code.c_str());
        needsRedraw = false;
        // Serial.println("[VIEW] ERROR redraw");
    }
}

// 機体0件
void handleNoFlightsView() {
    // ------------------------------------------------------
    // ボタン処理（毎回実行）
    // ------------------------------------------------------
    if (btnBWasPressed()) {
        // RETRY：確認ダイアログを挟まず、そのまま再取得へ
        currentMode = MODE_LOADING;
        needsRedraw = true;
        // Serial.printf("[BTN] BtnB wasPressed. currentMode = %d\n", currentMode);
        return;
    }

    if (btnCWasPressed()) {
        // SET：設定メニュー画面へ（機体0件画面経由として記録）
        menuCaller = MENUCALLER_NOFLIGHT;
        currentMode = MODE_MENU_VIEW;
        cursorIndex = 0;
        needsRedraw = true;
        // Serial.printf("[BTN] BtnC wasPressed. currentMode = %d\n", currentMode);
        return;
    }

    // ------------------------------------------------------
    // 描画処理（needsRedrawがtrueの時のみ実行）
    // ------------------------------------------------------
    if (needsRedraw) {
        drawNoFlightsView();
        needsRedraw = false;
        // Serial.println("[VIEW] NO FLIGHTS redraw");
    }
}

// データ取得中
// 1回の呼び出し内でWi-Fi接続〜APIリクエスト〜解析まで完結させる
//（操作可能なボタンは配置しておらず処理中に入力を受け付ける必要はない）
void handleLoadingView() {
    // ------------------------------------------------------
    // 1. Wi-Fi接続
    // ------------------------------------------------------
    drawLoadingScreen("Connecting to Wi-Fi...");
    enableWiFi();

    if (!tryConnectWiFi()) {
        // Wi-Fi接続失敗：Wi-Fi再接続フローの通知画面へ遷移する
        disableWiFi();
        currentMode = MODE_CONNECTION_FAILED;
        needsRedraw = true;
        Serial.println("[LOADING] Wi-Fi connection failed");
        return;
    }

    // 接続成功時、現在日時を同期する（失敗してもlastUpdateTimeは前回値を保持したまま処理を続行）
    struct tm timeInfo;
    if (syncTime(timeInfo)) {
        lastUpdateTime = formatUpdateTime(timeInfo);
    }
        
    Serial.print("[LOADING] Wi-Fi connected(");
    Serial.print(WiFi.localIP().toString());
    Serial.println(")");

    // ------------------------------------------------------
    // 2. APIリクエスト
    // ------------------------------------------------------
    drawLoadingScreen("Fetching flight data...");
    String rawJson;

    // HTTP通信エラー時の処理
    if (!fetchFlightsRaw(rawJson)) {
        disableWiFi();
        currentError.message = "HTTP request failed";
        currentError.code = "http_error";
        currentMode = MODE_ERROR_VIEW;
        needsRedraw = true;
        Serial.println("[LOADING] fetchFlightsRaw() failed");
        return;
    }

    Serial.println("[LOADING] fetchFlightsRaw() done");

    // ------------------------------------------------------
    // 3. レスポンス解析
    // ------------------------------------------------------
    drawLoadingScreen("Processing...");
    int remainingRequests = 0;
    ErrorData parseError;
    bool parseOk = parseFlightsResponse(rawJson, foundFlights, totalFlightCount, remainingRequests, parseError);

    disableWiFi();                          // 通信不要な処理（キャッシュ保存・画面遷移）の前に切断し、低消費電力運用に戻す

    // JSON解析失敗時の処理
    if (!parseOk) {
        currentError = parseError;
        currentMode = MODE_ERROR_VIEW;
        needsRedraw = true;
        Serial.println("[LOADING] parseFlightsResponse() failed");
        return;
    }

    Serial.printf("[LOADING] Fetch+Parse flights: %d, remainingRequests: %d\n", totalFlightCount, remainingRequests);

    // ------------------------------------------------------
    // 4. キャッシュ保存・画面遷移（0件はエラーではないためNO_FLIGHTS_VIEWへ）
    // ------------------------------------------------------
    saveCache(foundFlights, totalFlightCount, remainingRequests);
    currentDisplayIndex = 0;                // 1機目から表示する

    currentMode = (totalFlightCount > 0) ? MODE_FLIGHT_VIEW : MODE_NO_FLIGHTS_VIEW;
    needsRedraw = true;
    Serial.printf("[LOADING] done. flights = %d, remainingRequests = %d\n", totalFlightCount, remainingRequests);
}

// Wi-Fi接続失敗の通知（データ再取得時）
void handleConnectionFailedView() {
    // ------------------------------------------------------
    // ボタン処理（毎回実行）
    // ------------------------------------------------------
    if (btnCWasPressed()) {
        // Wi-Fi：Wi-Fi再設定の確認ダイアログへ遷移する
        currentMode = MODE_CONFIRM_DIALOG;
        currentConfirm = CONFIRM_WIFI_RECONNECT;
        needsRedraw = true;
        Serial.printf("[BTN] BtnC wasPressed. currentMode = %d, currentConfirm = %d\n", currentMode, currentConfirm);
        return;
    }

    // ------------------------------------------------------
    // 描画処理（needsRedrawがtrueの時のみ実行）
    // ------------------------------------------------------
    if (needsRedraw) {
        drawConnectionFailedView();
        needsRedraw = false;
    }
}


// ============================================================
// SCAN RANGE画面のカーソル初期化処理（現在の設定値の位置に合わせる）
// ============================================================
void resetScanRangeCursor() {
    ConfigData config;
    loadConfig(config);
    scanRangeCursorIndex = (config.scanRange == "WIDE") ? 1 : 0;
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

    // Serial.println("[RESET] All settings cleared. Restarting...");

    // 再起動の予告を表示（利用者に処理中であることを伝えるため、表示秒数とdelay()を一致させる）
    drawLoadingScreen("Restarting in 3 seconds");
    delay(3000);

    // 実機再起動（全グローバル変数・静的変数が確実に初期状態へ戻る）
    ESP.restart();
}