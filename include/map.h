#pragma once
#include "world.h"

int load_map_and_init(State *S, const char *map_file); // 0=ok, <0=err
int load_random_map(State *S, int r, int c);
