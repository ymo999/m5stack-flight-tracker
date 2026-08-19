/**
 * @file error_data.h
 * @brief エラー画面表示用データ構造体・定数定義
 */

#ifndef ERROR_DATA_H                    // インクルードガード（二重定義防止）
#define ERROR_DATA_H

#include <Arduino.h>

/**
 * @brief エラー画面に表示するメッセージ・コードのペア
 */
struct ErrorData {
    String message;                     ///< レスポンスのerror.message（またはHTTP/パースエラー時の自前文字列）
    String code;                        ///< レスポンスのerror.code（同上）
};

/**
 * @brief MODE_ERROR_VIEW遷移前にセットする、表示用のエラー内容
 */
extern ErrorData currentError;

#endif