#include <stdlib.h>
#include <time.h>
#include "world.h"

int load_random_map(State *S, int r, int c){
    if (r > MAX_ROWS) r = MAX_ROWS;
    if (c > MAX_COLS) c = MAX_COLS;
    S->rows = r;
    S->cols = c;
    srand((unsigned)time(NULL));
    for (int i=0;i<r;i++){
        for (int j=0;j<c;j++){
            int v = (rand()%100 < 20) ? WALL : PATH;
            S->maze[i][j] = v;
        }
    }
    S->car.x = 1; S->car.y = 1; S->car.direction = RIGHT;
    S->maze[1][1] = START;
    S->target_cnt = 0;
    return 0;
}