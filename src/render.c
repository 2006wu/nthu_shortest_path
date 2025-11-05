#include "render.h"
#include <stdio.h>
#include <unistd.h>

static void clear_screen(void){
    printf("\033[2J\033[H");
}

int g_delay_us = 100000;

void render_map(const State *S){
    clear_screen();
    int H = S->car_h;
    int W = S->car_w;

    int cx = S->car.x;
    int cy = S->car.y;
    int d  = S->car.direction;

    for (int i = 0; i < S->rows; i++) {
        for (int j = 0; j < S->cols; j++) {
            int printed = 0;

            if (d == UP) {
                if (i == cx && j == cy) {
                    printf("🫥"); printed = 1;
                } else if (i >= cx && i < cx + H && j >= cy && j < cy + W) {
                    printf("🛹"); printed = 1;
                }
            } else if (d == DOWN) {
                if (i == cx && j == cy) {
                    printf("🫥"); printed = 1;
                } else if (i <= cx && i > cx - H && j <= cy && j > cy - W) {
                    printf("🛹"); printed = 1;
                }
            } else if (d == RIGHT) {
                // 頭在 (cx,cy)~(cx+1,cy) 不變
                if ((i == cx || i == cx+1) && j == cy) {
                    printf("🫥"); printed = 1;
                } else if (i >= cx && i < cx + 2 && j <= cy && j > cy - H) {
                    // 寬固定 2，高用 H
                    printf("🛹"); printed = 1;
                }
            } else { // LEFT
                if ((i == cx || i == cx-1) && j == cy) {
                    printf("🫥"); printed = 1;
                } else if (i <= cx && i > cx - 2 && j >= cy && j < cy + H) {
                    printf("🛹"); printed = 1;
                }
            }

            if (printed) continue;

            int v = S->maze[i][j];
            if (v == 0) printf("⬛");
            else if (v == 1) printf("⬜");
            else if (v == 2) printf("🟩");
            else if (v == 3) printf("🟥");
            else if (v == 4) printf("🟦");
            else if (v == 5) printf("🟧");
            else if (v == 6) printf("🟫");
            else if (v == 7) printf("🟪");
            else if (v == 9) printf("🔳");
            else printf("  ");
        }
        printf("\n");
    }
    usleep(g_delay_us);
}
