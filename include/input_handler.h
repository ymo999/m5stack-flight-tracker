/**
 * @file input_handler.h
 * @brief ボタン入力関連の関数宣言
 * 
 * CoreS3等、物理ボタンを持たない機種でのタッチ操作対応を担う
 */

#ifndef INPUT_HANDLER_H                 // インクルードガード（二重定義防止）
#define INPUT_HANDLER_H

/**
 * @brief タッチ座標をM5.BtnA/B/Cの状態へ変換する（CoreS3等、対象機種でのみ動作）
 * 
 * @note main.cppのloop()内、M5.update()の直後に呼び出すこと
 */
void updateTouchButtons();


/**
 * @brief 各ボタンが「押された瞬間」かどうかを返す（物理・仮想ボタン両対応）
 * 
 * @note state_machine.cpp等は M5.BtnX.wasPressed() の代わりにこれらを呼び出すこと
 */
bool btnAWasPressed();                  // ボタンAが押されたかどうか
bool btnBWasPressed();                  // ボタンBが押されたかどうか
bool btnCWasPressed();                  // ボタンCが押されたかどうか

#endif