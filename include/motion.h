#pragma once
#include <stdbool.h>
#include "world.h"

/* 檢查並往目前方向前進一格，成功會改 S->car，失敗不動 */
int forward_if_free(State *S);

/* 左轉，成功會改 S->car，失敗不動 */
int turn_left_if_free(State *S);

/* 右轉，成功會改 S->car，失敗不動 */
int turn_right_if_free(State *S);

/* 前後調換，成功會改 S->car，失敗不動 */
int uturn_if_free(State *S);

/* 判斷車身 3×2 是否碰到 target 那一格 */
bool touch_target(const State *S, int tx, int ty, int x, int y, int dir);