#include "map.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void reset_state(State *S){
    memset(S, 0, sizeof(*S));
}

int load_map_and_init(State *S, const char *map_file){
    reset_state(S);

    FILE *file = fopen(map_file, "r");
    if (!file) {
        fprintf(stderr, "Unable to open file: %s\n", map_file);
        return -1;
    }

    char line[2048];
    int r = 0, cmax = 0;
    while (r < MAX_ROWS && fgets(line, sizeof(line), file)) {
        int c = 0;
        for (int i = 0; line[i] && line[i] != '\n' && c < MAX_COLS; i++) {
            if (line[i]>='0' && line[i]<='9') {
                S->maze[r][c++] = line[i] - '0';
            }
        }
        if (cmax < c) cmax = c;
        r++;
    }
    fclose(file);

    S->rows = r;
    S->cols = cmax;

    // 尋找 target 與起始車輛
    for (int i=0;i<S->rows;i++){
        for (int j=0;j<S->cols;j++){
            if (S->maze[i][j] == TARGET){
                if (S->target_cnt < (int)(sizeof(S->targets)/sizeof(S->targets[0]))){
                    S->targets[S->target_cnt].x = i;
                    S->targets[S->target_cnt].y = j;
                    S->targets[S->target_cnt].visited = false;
                    S->target_cnt++;
                }
            }
        }
    }

    // 依你原始偵測規則找車輛（兩種圖樣）
    for (int i=0;i<S->rows;i++){
        for (int j=0;j<S->cols;j++){
            if (S->maze[i][j] == START){
                if (i+1<S->rows && j+2<S->cols &&
                    S->maze[i+1][j] == START &&
                    S->maze[i][j+1] == START &&
                    S->maze[i][j+2] == START){
                    S->car.x = i;
                    S->car.y = j+2;
                    S->car.direction = RIGHT;
                    goto done_car;
                } else if (i+2<S->rows && j+1<S->cols &&
                           S->maze[i+1][j] == START &&
                           S->maze[i+2][j] == START &&
                           S->maze[i][j+1] == START){
                    S->car.x = i;
                    S->car.y = j;
                    S->car.direction = UP;
                    goto done_car;
                }
            }
        }
    }
done_car:

    S->path_step = 0;
    return 0;
}
