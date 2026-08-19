/**
 * input_handler.cpp
 * ボタン入力関連の処理
 * CoreS3等、物理ボタンを持たない機種向けに、タッチ座標から仮想ボタンの
 * 押下状態を独自管理し、物理ボタン（M5.BtnA/B/C）と統合した判定関数を提供する
 */

#include "input_handler.h"

// M5Unified.hはui_handler.hの中でインクルード済

#include "system_status.h"                  // 物理ボタン有無判定
#include "ui_handler.h"                     // BUTTON_AREA_*定数（描画側と共通化）

// ============================================================
// 仮想ボタン（タッチ）の状態管理
// ============================================================
// 前回フレームでの押下状態（A/B/Cそれぞれ）
static bool prevTouchPressed[3] = { false, false, false };
// 今回フレームで新規に押されたか（「押されていない→押されている」に変化した瞬間のみtrue）
static bool touchJustPressed[3] = { false, false, false };

// ============================================================
// タッチ座標から仮想ボタンA/B/Cの押下状態を更新する
// ============================================================
/**
 * drawButtonLabels()のボタンエリア（区切り線より下）のタッチを対象とし、
 * X座標を3分割してA/B/Cに割り当てる。
 * M5.BtnA等のButton_Classは経由せず、押下状態をこのファイル内で独自管理する
 * （Button_Class.setRawState()経由では正しく反映されない事象が確認されたため）。
 */
void updateTouchButtons() {

    // ------------------------------------------------------
    // 1. 非対象機種の早期リターン（Basic等は物理ボタン）
    //    M5.Touch系APIを誤って呼ばないよう、機種判定を先に行う
    // ------------------------------------------------------
    if (!isVirtualButtonBoard()) {
        return;
    }
    if (!M5.Touch.isEnabled()) {
        return;
    }

    // ------------------------------------------------------
    // 2. タッチ座標を取得し、ボタンエリア（区切り線より下）のみを対象にする
    // ------------------------------------------------------
    bool nowPressed[3] = { false, false, false };                               // 今回のループでのタッチ状態を格納

    int touchCount = M5.Touch.getCount();                                       // 検出されたタッチポイントの数
    for (int i = 0; i < touchCount; i++) {
        auto raw = M5.Touch.getTouchPointRaw(i);                                // タッチポイントの情報
        if (raw.y < BUTTON_AREA_Y) {
            continue;                                                           // ボタンエリア外のタッチは無視
        }

        auto detail = M5.Touch.getDetail(i);                                    // タッチ状態の詳細情報（touch：触れている、hold：長押し、flick：フリック、など）
        if (detail.state & m5::touch_state_t::touch) {                          // タッチされていれば（ビットAND演算）
            int index = (raw.x - BUTTON_AREA_MARGIN) / BUTTON_AREA_WIDTH;       // indexを算出（0～2の3つのindexがボタンA/B/Cに対応）
            if (index >= 0 && index <= 2) {
                nowPressed[index] = true;
            }
        }
    }

    // ------------------------------------------------------
    // 3. 前回状態との比較で「押された瞬間」を検知
    // ------------------------------------------------------
    for (int i = 0; i < 3; i++) {
        touchJustPressed[i] = nowPressed[i] && !prevTouchPressed[i];            // 「押されていない → 押された」でtrue
        prevTouchPressed[i] = nowPressed[i];
    }
}

// ============================================================
// 物理ボタン・仮想ボタンを統合した判定関数
// ============================================================
// state_machine.cpp等はM5.BtnX.wasPressed()の代わりにこれらを呼び出す
// ※複数のボタンエリアを同時にタップすると複数のbtn～WasPressedがtrueを返すが、
// 呼び出し元は「btnA → btnB → btnC」の順に判定しているため、左側が優先される
bool btnAWasPressed() { return M5.BtnA.wasPressed() || touchJustPressed[0]; }
bool btnBWasPressed() { return M5.BtnB.wasPressed() || touchJustPressed[1]; }
bool btnCWasPressed() { return M5.BtnC.wasPressed() || touchJustPressed[2]; }
