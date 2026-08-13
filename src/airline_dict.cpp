/**
 * airline_dict.cpp
 * 航空会社辞書（ICAOコード→名称変換）の実装
 */

#include "airline_dict.h"

#include <string.h>

// 航空会社辞書データ（暫定・主要10社のみ。手順29で主要50〜100社へ拡充予定。
// ICAOコードの昇順にソート済みであること（二分探索の前提条件）
static const AirlineEntry AIRLINE_TABLE[] = {
    {"AAL", "American Airlines"},
    {"ACA", "Air Canada"},
    {"ANA", "All Nippon Airways"},
    {"CPA", "Cathay Pacific Airways"},
    {"DAL", "Delta Airlines"},
    {"EVA", "Eva Airway"},
    {"JAL", "Japan Airlines"},
    {"SIA", "Singapore Airlines"},
    {"SKY", "Skymark Airlines"},
    {"UAL", "United Airlines"},
};

static const int AIRLINE_COUNT = sizeof(AIRLINE_TABLE) / sizeof(AirlineEntry);

// ============================================================
// ICAOコードから航空会社名を取得する（二分探索）
// ============================================================
const char* getAirlineName(const char* icaoCode) {
    int low = 0;
    int high = AIRLINE_COUNT - 1;

    while (low <= high) {
        int mid = (low + high) / 2;
        int cmp = strcmp(icaoCode, AIRLINE_TABLE[mid].code);

        if (cmp == 0) {
            return AIRLINE_TABLE[mid].name;
        } else if (cmp < 0) {
            high = mid - 1;
        } else {
            low = mid + 1;
        }
    }

    // 見つからない場合はコードをそのまま返す（フォールバック）
    return icaoCode;
}