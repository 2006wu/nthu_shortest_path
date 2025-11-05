#include "stdio.h"
#include "stdlib.h"
#include "unistd.h"
#include "fcntl.h"
#include "render.h"
#include "algo.h"
#include "motion.h"

void set_stdin_nonblock(void){
    int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);
}

void reset_stdin_block(void){
    int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, flags & ~O_NONBLOCK);
}

// 排列好heap由距離短到長
typedef struct Node{
    int x, y, dir;
    int dist;
} Node;

static Node heap[MAX_ROWS*MAX_COLS*4 + 5];
static int hsz = 0;

static void hclear(){ hsz = 0; }
static void hpush(Node v){
    int i = ++hsz;
    heap[i] = v;
    while(i>1 && heap[i].dist < heap[i/2].dist){
        Node t = heap[i];
        heap[i] = heap[i/2];
        heap[i/2] = t;
        i = i/2;
    }
}
static int hempty(){ return hsz == 0; }
static Node hpop(void){
    Node res = heap[1];
    heap[1] = heap[hsz--];
    int i = 1;
    while(1){
        int l = i*2, rr = i*2+1, m = i;
        if (l <= hsz && heap[l].dist < heap[m].dist) m = l;
        if (rr <= hsz && heap[rr].dist < heap[m].dist) m = rr;
        if (m == i) break;
        Node t = heap[i];
        heap[i] = heap[m];
        heap[m] = t;
        i = m;
    }
    return res;
}

// paint path
static void paint_car_pose(State *S, int x, int y, int dir){
    // 1*1直接畫一格
    if (S->car_h == 1 && S->car_w == 1) {
        int i = x, j = y;
        if (i >= 0 && i < S->rows && j >= 0 && j < S->cols) {
            if (S->maze[i][j] != WALL && S->maze[i][j] != TARGET)
                S->maze[i][j] = (replay_cnt % 2 == 0) ? PASS : RE_PASS;
        }
        return;
    }

    // 2*2 or original car
    int H = S->car_h;
    int rows = S->rows;
    int cols = S->cols;

    int x1, x2, y1, y2;
    if (dir == UP) {
        x1 = x; x2 = x + H - 1;
        y1 = y; y2 = y + 1;
    } else if (dir == DOWN) {
        x1 = x - H + 1; x2 = x;
        y1 = y - 1; y2 = y;
    } else if (dir == RIGHT) {
        x1 = x; x2 = x + 1;
        y1 = y - H + 1; y2 = y;
    } else { // LEFT
        x1 = x - 1; x2 = x;
        y1 = y; y2 = y + H - 1;
    }

    for (int i = x1; i <= x2; i++) {
        for (int j = y1; j <= y2; j++) {
            if (i < 0 || i >= rows || j < 0 || j >= cols) continue;
            if (S->maze[i][j] == TARGET || S->maze[i][j] == REACH) continue;

            // 重播就改變原本路徑顏色
            if (replay_cnt % 2 == 1)
                S->maze[i][j] = RE_PASS;
            else if (replay_cnt % 2 == 0)
                S->maze[i][j] = PASS;
        }
    }
}

// Dijkstra
static int dist[MAX_ROWS][MAX_COLS][4];
static int px[MAX_ROWS][MAX_COLS][4];
static int py[MAX_ROWS][MAX_COLS][4];
static int pd[MAX_ROWS][MAX_COLS][4];

// first_version : greedy the nearest target and dijkstra to it
int dijkstra_one_target(State *S, int target){
    // init distances
    for (int i=0;i<S->rows;i++){
        for (int j=0;j<S->cols;j++){
            for (int d=0;d<4;d++){
                dist[i][j][d] = -1;
                px[i][j][d] = -1;
                py[i][j][d] = -1;
                pd[i][j][d] = -1;
            }
        }
    }
    hclear();

    // push start state
    int sx = S->car.x, sy = S->car.y, sd = S->car.direction;
    dist[sx][sy][sd] = 0;
    hpush((Node){sx, sy, sd, 0});

    while(!hempty()){
        Node u = hpop();
        if (dist[u.x][u.y][u.dir] < u.dist) continue;

        // reach target
        if (touch_target(S, S->targets[target].x, S->targets[target].y, u.x, u.y, u.dir)){
            // reconstruct path
            int cx = u.x, cy = u.y, cd = u.dir;
            int n = 0;
            while(cx != -1){
                S->result_path[n][0] = cx;
                S->result_path[n][1] = cy;
                S->result_path[n][2] = cd;
                n++;
                int nx = px[cx][cy][cd];
                int ny = py[cx][cy][cd];
                int nd = pd[cx][cy][cd];
                cx = nx; cy = ny; cd = nd;
            }
            // reverse path
            for (int i=0;i<n/2;i++){
                int j = n-1-i;
                int t0 = S->result_path[i][0];
                int t1 = S->result_path[i][1];
                int t2 = S->result_path[i][2];
                S->result_path[i][0] = S->result_path[j][0];
                S->result_path[i][1] = S->result_path[j][1];
                S->result_path[i][2] = S->result_path[j][2];
                S->result_path[j][0] = t0;
                S->result_path[j][1] = t1;
                S->result_path[j][2] = t2;
            }
            S->path_step = n;
            // set car to target state
            S->car.x = u.x; S->car.y = u.y; S->car.direction = u.dir;
            return 0;
        }

        // try all motions
        Car saved = S->car; // save current car state
        // forward
        S->car = saved;
        S->car.x = u.x; S->car.y = u.y; S->car.direction = u.dir;
        if (forward_if_free(S)){
            int nx = S->car.x, ny = S->car.y, nd = S->car.direction;
            int newd = u.dist + 1;
            if (dist[nx][ny][nd] == -1 || dist[nx][ny][nd] > newd){
                dist[nx][ny][nd] = newd;
                px[nx][ny][nd] = u.x;
                py[nx][ny][nd] = u.y;
                pd[nx][ny][nd] = u.dir;
                hpush((Node){nx, ny, nd, newd});
            }
        }

        // turn left
        S->car = saved;
        S->car.x = u.x; S->car.y = u.y; S->car.direction = u.dir;
        if (turn_left_if_free(S)){
            int nx = S->car.x, ny = S->car.y, nd = S->car.direction;
            int newd = u.dist + 3;
            if (dist[nx][ny][nd] == -1 || dist[nx][ny][nd] > newd){
                dist[nx][ny][nd] = newd;
                px[nx][ny][nd] = u.x;
                py[nx][ny][nd] = u.y;
                pd[nx][ny][nd] = u.dir;
                hpush((Node){nx, ny, nd, newd});
            }
        }

        // turn right
        S->car = saved;
        S->car.x = u.x; S->car.y = u.y; S->car.direction = u.dir;
        if (turn_right_if_free(S)){
            int nx = S->car.x, ny = S->car.y, nd = S->car.direction;
            int newd = u.dist + 3;
            if (dist[nx][ny][nd] == -1 || dist[nx][ny][nd] > newd){
                dist[nx][ny][nd] = newd;
                px[nx][ny][nd] = u.x;
                py[nx][ny][nd] = u.y;
                pd[nx][ny][nd] = u.dir;
                hpush((Node){nx, ny, nd, newd});
            }
        }

        // uturn
        S->car = saved;
        S->car.x = u.x; S->car.y = u.y; S->car.direction = u.dir;
        if (uturn_if_free(S)){
            int nx = S->car.x, ny = S->car.y, nd = S->car.direction;
            int newd = u.dist + 3;
            if (dist[nx][ny][nd] == -1 || dist[nx][ny][nd] > newd){
                dist[nx][ny][nd] = newd;
                px[nx][ny][nd] = u.x;
                py[nx][ny][nd] = u.y;
                pd[nx][ny][nd] = u.dir;
                hpush((Node){nx, ny, nd, newd});
            }
        }

        S->car = saved; // restore car state
    }
    return -1; // unreachable
}

int greedy_run_all_target(State *S, int reply_mode){
    if (reply_mode == 1){
        set_stdin_nonblock();
    }
    int done = 0;
    int unreachable[30];
    int unreachable_cnt = 0;
    while(1){
        int best = -1;
        int best_dist = 1e9;
        // find nearest reachable target
        for (int t=0;t<S->target_cnt;t++){
            if (S->targets[t].visited) continue;
            int d = abs(S->car.x - S->targets[t].x) + abs(S->car.y - S->targets[t].y);
            if (d < best_dist){ best_dist = d; best = t; }
        }
        if (best < 0) break; // all done

        if (dijkstra_one_target(S, best) == 0){
            int first = 1;
            // int first_press = 1;
            for (int i=0;i<S->path_step;i++){
                int px = S->result_path[i][0];
                int py = S->result_path[i][1];
                int pd = S->result_path[i][2];

                // S->maze[px][py] = PASS;
                paint_car_pose(S, px, py, pd);

                S->car.x = px;
                S->car.y = py;
                S->car.direction = pd;

                if (S->runlog.count < MAX_LOG_STEPS) {
                    S->runlog.steps[S->runlog.count].x = px;
                    S->runlog.steps[S->runlog.count].y = py;
                    S->runlog.steps[S->runlog.count].d = pd;
                    S->runlog.count++;
                }

                render_map(S);
                if (reply_mode == 2){
                    if (first) {
                        int ch;
                        while((ch = getchar()) != '\n' && ch != EOF);
                        first = 0;
                    }
                    // if (first_press) {
                    //     printf("press enter to continue...");
                    //     first_press = 0;
                    // }
                    printf("press enter to continue...");
                    getchar();
                }
            }

            int tx = S->targets[best].x;
            int ty = S->targets[best].y;
            S->maze[tx][ty] = REACH;
            
            S->targets[best].visited = true;
            done++;
        } else {
            S->targets[best].visited = true;
            unreachable[unreachable_cnt++] = best;
        }
    }

    for (int i=0;i<unreachable_cnt;i++){
        int k = unreachable[i];
        printf("Unreachable target:(%d,%d)\n", S->targets[k].x, S->targets[k].y);
    }

    return done;
}

// Second version : run dijkstra to the first unvisited

int dijkstra_first_unvisited(State *S){
    // init distances
    for (int i=0;i<S->rows;i++){
        for (int j=0;j<S->cols;j++){
            for (int d=0;d<4;d++){
                dist[i][j][d] = -1;
                px[i][j][d] = -1;
                py[i][j][d] = -1;
                pd[i][j][d] = -1;
            }
        }
    }
    hclear();

    // push start state
    int sx = S->car.x, sy = S->car.y, sd = S->car.direction;
    dist[sx][sy][sd] = 0;
    hpush((Node){sx, sy, sd, 0});

    while(!hempty()){
        Node u = hpop();
        if (dist[u.x][u.y][u.dir] < u.dist) continue;

        for (int k=0;k<S->target_cnt;k++){
            if (S->targets[k].visited) continue;
            // reach target
            if (touch_target(S, S->targets[k].x, S->targets[k].y, u.x, u.y, u.dir)){
                // reconstruct path
                int cx = u.x, cy = u.y, cd = u.dir;
                int n = 0;
                while(cx != -1){
                    S->result_path[n][0] = cx;
                    S->result_path[n][1] = cy;
                    S->result_path[n][2] = cd;
                    n++;
                    int nx = px[cx][cy][cd];
                    int ny = py[cx][cy][cd];
                    int nd = pd[cx][cy][cd];
                    cx = nx; cy = ny; cd = nd;
                }
                // reverse path
                for (int i=0;i<n/2;i++){
                    int j = n-1-i;
                    int t0 = S->result_path[i][0];
                    int t1 = S->result_path[i][1];
                    int t2 = S->result_path[i][2];
                    S->result_path[i][0] = S->result_path[j][0];
                    S->result_path[i][1] = S->result_path[j][1];
                    S->result_path[i][2] = S->result_path[j][2];
                    S->result_path[j][0] = t0;
                    S->result_path[j][1] = t1;
                    S->result_path[j][2] = t2;
                }
                S->path_step = n;
                // set car to target state
                S->car.x = u.x; S->car.y = u.y; S->car.direction = u.dir;
                return k;
            }
        }
        // try all motions
        Car saved = S->car; // save current car state
        // forward
        S->car = saved;
        S->car.x = u.x; S->car.y = u.y; S->car.direction = u.dir;
        if (forward_if_free(S)){
            int nx = S->car.x, ny = S->car.y, nd = S->car.direction;
            int newd = u.dist + 1;
            if (dist[nx][ny][nd] == -1 || dist[nx][ny][nd] > newd){
                dist[nx][ny][nd] = newd;
                px[nx][ny][nd] = u.x;
                py[nx][ny][nd] = u.y;
                pd[nx][ny][nd] = u.dir;
                hpush((Node){nx, ny, nd, newd});
            }
        }

        // turn left
        S->car = saved;
        S->car.x = u.x; S->car.y = u.y; S->car.direction = u.dir;
        if (turn_left_if_free(S)){
            int nx = S->car.x, ny = S->car.y, nd = S->car.direction;
            int newd = u.dist + 3;
            if (dist[nx][ny][nd] == -1 || dist[nx][ny][nd] > newd){
                dist[nx][ny][nd] = newd;
                px[nx][ny][nd] = u.x;
                py[nx][ny][nd] = u.y;
                pd[nx][ny][nd] = u.dir;
                hpush((Node){nx, ny, nd, newd});
            }
        }

        // turn right
        S->car = saved;
        S->car.x = u.x; S->car.y = u.y; S->car.direction = u.dir;
        if (turn_right_if_free(S)){
            int nx = S->car.x, ny = S->car.y, nd = S->car.direction;
            int newd = u.dist + 3;
            if (dist[nx][ny][nd] == -1 || dist[nx][ny][nd] > newd){
                dist[nx][ny][nd] = newd;
                px[nx][ny][nd] = u.x;
                py[nx][ny][nd] = u.y;
                pd[nx][ny][nd] = u.dir;
                hpush((Node){nx, ny, nd, newd});
            }
        }

        // uturn
        S->car = saved;
        S->car.x = u.x; S->car.y = u.y; S->car.direction = u.dir;
        if (uturn_if_free(S)){
            int nx = S->car.x, ny = S->car.y, nd = S->car.direction;
            int newd = u.dist + 3;
            if (dist[nx][ny][nd] == -1 || dist[nx][ny][nd] > newd){
                dist[nx][ny][nd] = newd;
                px[nx][ny][nd] = u.x;
                py[nx][ny][nd] = u.y;
                pd[nx][ny][nd] = u.dir;
                hpush((Node){nx, ny, nd, newd});
            }
        }

        S->car = saved; // restore car state
    }
    return -1; // unreachable
}

int run_all_target_first_unvisited(State *S, int reply_mode){
    if (reply_mode == 1) {
        set_stdin_nonblock();
    }
    int reachable[30], rcnt = 0;
    int unreachable[30], ucnt = 0;

    while(1){
        int k = dijkstra_first_unvisited(S);
        if (k < 0){
            for (int i=0;i<S->target_cnt;i++){
                if (!S->targets[i].visited){
                    S->targets[i].visited = 1;
                    unreachable[ucnt++] = i;
                }
            }
            break;
        }

        // mark target as visited
        for (int i=0;i<S->path_step;i++){
            int px = S->result_path[i][0];
            int py = S->result_path[i][1];
            int pd = S->result_path[i][2];

            // S->maze[px][py] = PASS;
            paint_car_pose(S, px, py, pd);
            S->car.x = px;
            S->car.y = py;
            S->car.direction = pd;
            render_map(S);

            // log the step
            if (S->runlog.count < MAX_LOG_STEPS){
                S->runlog.steps[S->runlog.count].x = px;
                S->runlog.steps[S->runlog.count].y = py;
                S->runlog.steps[S->runlog.count].d = pd;
                S->runlog.count++;
            }

            /* control the speed */
            if (reply_mode == 1){
                char c;
                if (read(STDIN_FILENO, &c, 1) == 1){
                    if (c == 'h'){
                        if (g_delay_us > 20000) g_delay_us -= 20000;
                    } else if (c == 'l') {
                        g_delay_us += 20000;
                    } else if (c == 'b') {
                        int fx = S->car.x, fy = S->car.y;
                        if (S->car.direction == UP) fx--;
                        else if (S->car.direction == DOWN) fx++;
                        else if (S->car.direction == LEFT) fy--;
                        else if (S->car.direction == RIGHT) fy++;
                        if (fx>=0 && fx<S->rows && fy>=0 && fy<S->cols)
                            S->maze[fx][fy] = WALL;
                    }
                }
            }
            int first = 1;
            if (reply_mode == 2){
                if (first) {
                    int ch;
                    while((ch = getchar()) != '\n' && ch != EOF);
                    first = 0;
                }
                printf("press enter to continue...");
                getchar();
            }            
        }

        int tx = S->targets[k].x;
        int ty = S->targets[k].y;
        S->maze[tx][ty] = REACH;
        S->targets[k].visited = true;
        reachable[rcnt++] = k;
    }

    printf("Reached %d / %d targets\n", rcnt, S->target_cnt);
    if (rcnt){
        printf("Reachable targets:\n");
        for (int i=0;i<rcnt;i++){
            int k = reachable[i];
            printf("  #%d at (%d,%d)\n", k, S->targets[k].x, S->targets[k].y);
        }
    }
    if (ucnt){
        printf("Unreachable targets:\n");
        for (int i=0;i<ucnt;i++){
            int k = unreachable[i];
            printf("  #%d at (%d,%d)\n", k, S->targets[k].x, S->targets[k].y);
        }
    }
    return rcnt;
}

void replay_runlog(State *S){
    replay_cnt++;
    int old_delay = g_delay_us;
    g_delay_us = 50000;

    for (int i = 0; i < S->runlog.count; i++) {
        S->car.x = S->runlog.steps[i].x;
        S->car.y = S->runlog.steps[i].y;
        S->car.direction = S->runlog.steps[i].d;

        paint_car_pose(S, S->car.x, S->car.y, S->car.direction);
        render_map(S);
    }

    g_delay_us = old_delay;
}
