/**
 * @file input_handler.h
 * @brief ボタン入力関連の関数宣言
 * 
 * CoreS3等、物理ボタンを持たない機種でのタッチ操作対応を担う
 */

#ifndef INPUT_HANDLER_H                   // インクルードガード（二重定義防止）
#define INPUT_HANDLER_H

/**
 * @brief タッチ座標をM5.BtnA/B/Cの状態へ変換する（CoreS3等、対象機種でのみ動作）
 * 
 * @note main.cppのloop()内、M5.update()の直後に呼び出すこと
 */
void updateTouchButtons();

/**
 * @brief ボタンAが「押された瞬間」かどうかを返す（物理ボタン・仮想ボタン両対応）
 * 
 * @note state_machine.cpp等はM5.BtnA.wasPressed()の代わりにこれらを呼び出すこと
 * @return true 押された瞬間である
 * @return false それ以外
 */
bool btnAWasPressed();

/**
 * @brief ボタンBが「押された瞬間」かどうかを返す（物理ボタン・仮想ボタン両対応）
 * 
 * @note state_machine.cpp等はM5.BtnA.wasPressed()の代わりにこれらを呼び出すこと
 * @return true 押された瞬間である
 * @return false それ以外
 */
bool btnBWasPressed();

/**
 * @brief ボタンCが「押された瞬間」かどうかを返す（物理ボタン・仮想ボタン両対応）
 * 
 * @note state_machine.cpp等はM5.BtnA.wasPressed()の代わりにこれらを呼び出すこと
 * @return true 押された瞬間である
 * @return false それ以外
 */
bool btnCWasPressed();

#endif