/*
    機体データの変数定義
*/
#include "flight_data.h"

// グローバル変数の実体定義（宣言はflight_data.h参照）
FlightData foundFlights[MAX_FLIGHT_COUNT];
int totalFlightCount = 0;
int currentDisplayIndex = 0;