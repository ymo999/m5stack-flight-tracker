/**
 * ui_handler.h
 * UI描画関連の関数宣言
 */

#ifndef UI_HANDLER_H                        // インクルードガード（二重定義防止）
#define UI_HANDLER_H

#include <Arduino.h>

// 進行方向（度数、0〜360）を8方位の英字表記に変換する
const char* getDirectionLabel(float heading);

// 時刻情報をMM/DD HH:MM形式の文字列に整形する（取得日時の表示用）
String formatUpdateTime(const struct tm& timeInfo);

#endif