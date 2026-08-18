/**
 * input_handler.h
 * ボタン入力関連の関数宣言
 * CoreS3等、物理ボタンを持たない機種でのタッチ操作対応を担う
 */

#ifndef INPUT_HANDLER_H                   // インクルードガード（二重定義防止）
#define INPUT_HANDLER_H

// タッチ座標をM5.BtnA/B/Cの状態へ変換する（CoreS3等、対象機種でのみ動作）
// main.cppのloop()内、M5.update()の直後に呼び出すこと
void updateTouchButtons();

// ボタンが「押された瞬間」かどうかを返す（物理ボタン・仮想ボタン両対応）
// state_machine.cpp等はM5.BtnA.wasPressed()の代わりにこれらを呼び出すこと
bool btnAWasPressed();
bool btnBWasPressed();
bool btnCWasPressed();

#endif