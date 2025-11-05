#ifndef EDIT_MAP_H
#define EDIT_MAP_H

#include "world.h"

void edit_map(State *S);
int save_map_to_file(const State *S, const char *fname);

#endif
