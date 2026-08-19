/**
 * @file airline_dict.h
 * @brief 航空会社辞書（ICAOコード→名称変換）の宣言
 */

#ifndef AIRLINE_DICT_H                  // インクルードガード（二重定義防止）
#define AIRLINE_DICT_H

/**
 * @brief 航空会社辞書の1エントリ（ICAOコードと航空会社名の対応）
 */
struct AirlineEntry {
    const char* code;   ///< ICAOコード（3文字）
    const char* name;   ///< 航空会社名
};

/**
 * @brief ICAOコードから航空会社名を取得する
 * @param[in] icaoCode ICAOコード（3文字）
 * @return 対応する航空会社名（見つからない場合はICAOコードをそのまま返す）
 */
const char* getAirlineName(const char* icaoCode);

#endif