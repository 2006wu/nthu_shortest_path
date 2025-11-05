#include <string.h>
#include <unistd.h>
#include <stdio.h>

#include "scan.h"
#include "motion.h"
#include "render.h"

#ifndef PASS
#define PASS 4
#endif
#ifndef REACH
#define REACH 5
#endif
#ifndef BLOCKED
#define BLOCKED 6
#endif

/* 把這個姿勢的六格塗成 4，target(3) 和 reach(5) 不蓋 */
static int paint_car_pose(State *S, int x, int y, int d){
    int painted = 0;
    if (d == UP){
        for (int i = x; i <= x+2; i++){
            for (int j = y; j <= y+1; j++){
                if (i<0 || i>=S->rows || j<0 || j>=S->cols) continue;
                if (S->maze[i][j] == TARGET || S->maze[i][j] == REACH) continue;
                if (S->maze[i][j] != PASS){
                    S->maze[i][j] = PASS;
                    painted = 1;
                }
            }
        }
    } else if (d == RIGHT){
        for (int i = x; i <= x+1; i++){
            for (int j = y-2; j <= y; j++){
                if (i<0 || i>=S->rows || j<0 || j>=S->cols) continue;
                if (S->maze[i][j] == TARGET || S->maze[i][j] == REACH) continue;
                if (S->maze[i][j] != PASS){
                    S->maze[i][j] = PASS;
                    painted = 1;
                }
            }
        }
    } else if (d == DOWN){
        for (int i = x-2; i <= x; i++){
            for (int j = y-1; j <= y; j++){
                if (i<0 || i>=S->rows || j<0 || j>=S->cols) continue;
                if (S->maze[i][j] == TARGET || S->maze[i][j] == REACH) continue;
                if (S->maze[i][j] != PASS){
                    S->maze[i][j] = PASS;
                    painted = 1;
                }
            }
        }
    } else { // LEFT
        for (int i = x-1; i <= x; i++){
            for (int j = y; j <= y+2; j++){
                if (i<0 || i>=S->rows || j<0 || j>=S->cols) continue;
                if (S->maze[i][j] == TARGET || S->maze[i][j] == REACH) continue;
                if (S->maze[i][j] != PASS){
                    S->maze[i][j] = PASS;
                    painted = 1;
                }
            }
        }
    }
    return painted;
}

int run_scan_full(State *S){
    static int vis[MAX_ROWS][MAX_COLS][4];
    memset(vis, 0, sizeof(vis));

    typedef struct { int x,y,d; } Q;
    Q q[MAX_ROWS*MAX_COLS*4];
    int qh = 0, qt = 0;

    int sx = S->car.x, sy = S->car.y, sd = S->car.direction;
    vis[sx][sy][sd] = 1;
    q[qt++] = (Q){sx, sy, sd};

    while (qh < qt){
        Q u = q[qh++];

        /* 畫這一步 */
        int painted = paint_car_pose(S, u.x, u.y, u.d);

        /* 更新車子姿勢，讓 render 看得到 */
        S->car.x = u.x;
        S->car.y = u.y;
        S->car.direction = u.d;

        if (painted){
            render_map(S);
            usleep(60000);
        }

        Car saved = S->car;

        /* forward */
        S->car = saved;
        if (forward_if_free(S)){
            int nx = S->car.x, ny = S->car.y, nd = S->car.direction;
            if (!vis[nx][ny][nd]){
                vis[nx][ny][nd] = 1;
                q[qt++] = (Q){nx, ny, nd};
            }
        }

        /* turn left */
        S->car = saved;
        if (turn_left_if_free(S)){
            int nx = S->car.x, ny = S->car.y, nd = S->car.direction;
            if (!vis[nx][ny][nd]){
                vis[nx][ny][nd] = 1;
                q[qt++] = (Q){nx, ny, nd};
            }
        }

        /* turn right */
        S->car = saved;
        if (turn_right_if_free(S)){
            int nx = S->car.x, ny = S->car.y, nd = S->car.direction;
            if (!vis[nx][ny][nd]){
                vis[nx][ny][nd] = 1;
                q[qt++] = (Q){nx, ny, nd};
            }
        }

        /* u-turn 可要可不要，跟你 motion 一致 */
        S->car = saved;
        if (uturn_if_free(S)){
            int nx = S->car.x, ny = S->car.y, nd = S->car.direction;
            if (!vis[nx][ny][nd]){
                vis[nx][ny][nd] = 1;
                q[qt++] = (Q){nx, ny, nd};
            }
        }
    }

    /* 標記沒掃到的可走格 */
    int unreachable = 0;
    for (int i=0;i<S->rows;i++){
        for (int j=0;j<S->cols;j++){
            if (S->maze[i][j] == PATH || S->maze[i][j] == START){
                if (S->maze[i][j] != PASS){
                    S->maze[i][j] = BLOCKED;
                    unreachable++;
                }
            }
        }
    }

    render_map(S);
    printf("unreachable cells: %d\n", unreachable);
    return unreachable;
}
