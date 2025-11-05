#pragma once
#include "world.h"
void render_map(const State *S);
void render_path(State *S); // 依 S->result_path 播放並畫軌跡

extern int g_delay_us;