/**
 * airline_dict.h
 * 航空会社辞書（ICAOコード→名称変換）の宣言
 */

#ifndef AIRLINE_DICT_H                  // インクルードガード（二重定義防止）
#define AIRLINE_DICT_H

// 航空会社辞書の1エントリ（ICAOコードと航空会社名の対応）
struct AirlineEntry {
    const char* code;   // ICAOコード（3文字）
    const char* name;   // 航空会社名
};

// ICAOコードから航空会社名を取得する（見つからない場合はコードをそのまま返す）
const char* getAirlineName(const char* icaoCode);

#endif