#include "motion.h"

static inline int in_bounds(const State *S, int x, int y){
    return x >= 0 && x < S->rows && y >= 0 && y < S->cols && S->maze[x][y] != WALL;
}

int forward_if_free(State *S){
    // 1*1 car
    if (S->car_h == 1 && S->car_w == 1) {
        int nx = S->car.x, ny = S->car.y;
        if (S->car.direction == UP) nx--;
        else if (S->car.direction == DOWN) nx++;
        else if (S->car.direction == LEFT) ny--;
        else if (S->car.direction == RIGHT) ny++;

        if (nx < 0 || ny < 0 || nx >= S->rows || ny >= S->cols) return false;
        if (S->maze[nx][ny] == WALL) return false;

        S->car.x = nx;
        S->car.y = ny;
        return true;
    }

    // original car
    int x = S->car.x;
    int y = S->car.y;
    int d = S->car.direction;

    int nx = x, ny = y;
    int c1x, c1y, c2x, c2y;

    if (d == UP){
        c1x = x-1; c1y = y;
        c2x = x-1; c2y = y+1;
        nx = x-1; ny = y;
    } else if (d == RIGHT){
        c1x = x;   c1y = y+1;
        c2x = x+1; c2y = y+1;
        nx = x;   ny = y+1;
    } else if (d == DOWN){
        c1x = x+1; c1y = y;
        c2x = x+1; c2y = y-1;
        nx = x+1; ny = y;
    } else { // LEFT
        c1x = x;   c1y = y-1;
        c2x = x-1; c2y = y-1;
        nx = x;   ny = y-1;
    }

    if (in_bounds(S,c1x,c1y) && in_bounds(S,c2x,c2y)){
        S->car.x = nx;
        S->car.y = ny;
        return 1;
    }
    return 0;
}

int turn_right_if_free(State *S){
    // 1*1 car
    if (S->car_h == 1 && S->car_w == 1) {
        S->car.direction = (S->car.direction + 1) % 4;
        return true;
    }

    // 2*2 car
    if (S->car_h == 2 && S->car_w == 2){
        int x = S->car.x, y = S->car.y;
        int d = S->car.direction;
        int hx, hy;
        int nd = (d + 1) & 3;
        if (d == UP){
            hx = x; hy = y+1;
        } else if (d == RIGHT){
            hx = x+1; hy = y;
        } else if (d == DOWN){
            hx = x; hy = y-1;
        } else { // LEFT
            hx = x-1; hy = y;
        }
        S->car.x = hx;
        S->car.y = hy;
        S->car.direction = nd;
        return 1;
    }

    // original car
    int x = S->car.x;
    int y = S->car.y;
    int d = S->car.direction;

    int c1x, c1y, c2x, c2y;   // 要檢查的兩格
    int hx, hy;               // 轉完的車頭
    int nd = (d + 1) & 3;

    if (d == UP){
        // U -> R
        // (x,y)(x,y+1) -> (x,y+2)(x+1,y+2)
        c1x = x;   c1y = y+2;
        c2x = x+1; c2y = y+2;
        hx = x; hy = y+2;
    } else if (d == RIGHT){
        // R -> D
        // (x,y)(x+1,y) -> (x+2,y)(x+2,y-1)
        c1x = x+2; c1y = y;
        c2x = x+2; c2y = y-1;
        hx = x+2; hy = y;
    } else if (d == DOWN){
        // D -> L
        // (x,y)(x,y-1) -> (x,y-2)(x-1,y-2)
        c1x = x;   c1y = y-2;
        c2x = x-1; c2y = y-2;
        hx = x; hy = y-2;
    } else { // LEFT
        // L -> U
        // (x,y)(x-1,y) -> (x-2,y)(x-2,y+1)
        c1x = x-2; c1y = y;
        c2x = x-2; c2y = y+1;
        hx = x-2; hy = y;
    }

    if (in_bounds(S,c1x,c1y) && in_bounds(S,c2x,c2y)){
        S->car.x = hx;
        S->car.y = hy;
        S->car.direction = nd;
        return 1;
    }
    return 0;
}

int turn_left_if_free(State *S){
    // 1*1 car
    if (S->car_h == 1 && S->car_w == 1) {
        S->car.direction = (S->car.direction + 3) % 4;
        return true;
    }

    // 2*2 car
    if (S->car_h == 2 && S->car_w == 2){
        int x = S->car.x, y = S->car.y;
        int d = S->car.direction;
        int hx, hy;
        int nd = (d + 3) & 3;
        if (d == UP){
            hx = x+1; hy = y;
        } else if (d == RIGHT){
            hx = x; hy = y-1;
        } else if (d == DOWN){
            hx = x-1; hy = y;
        } else { // LEFT
            hx = x; hy = y+1;
        }
        S->car.x = hx;
        S->car.y = hy;
        S->car.direction = nd;
        return 1;
    }

    // original car
    int x = S->car.x;
    int y = S->car.y;
    int d = S->car.direction;

    int c1x, c1y, c2x, c2y;
    int hx, hy;
    int nd = (d + 3) & 3;

    if (d == UP){
        // U -> L
        // (x,y)(x,y+1) -> (x+1,y-1)(x,y-1)
        c1x = x+1; c1y = y-1;
        c2x = x;   c2y = y-1;
        hx = x+1; hy = y-1;
    } else if (d == RIGHT){
        // R -> U
        // (x,y)(x+1,y) -> (x-1,y-1)(x-1,y1)
        c1x = x-1; c1y = y-1;
        c2x = x-1; c2y = y;
        hx = x-1; hy = y-1;
    } else if (d == DOWN){
        // D -> R
        // (x,y)(x,y-1) -> (x-1,y+1)(x,y+1)
        c1x = x-1;   c1y = y+1;
        c2x = x; c2y = y+1;
        hx = x-1; hy = y+1;
    } else { // LEFT
        // L -> D
        // (x,y)(x-1,y) -> (x+1,y+1)(x+1,y)
        c1x = x+1; c1y = y+1;
        c2x = x+1; c2y = y;
        hx = x+1; hy = y+1;
    }

    if (in_bounds(S,c1x,c1y) && in_bounds(S,c2x,c2y)){
        S->car.x = hx;
        S->car.y = hy;
        S->car.direction = nd;
        return 1;
    }
    return 0;
}

int uturn_if_free(State *S){
    // 1*1 car
    if (S->car_h == 1 && S->car_w == 1) {
        S->car.direction = (S->car.direction + 2) % 4;
        return true;
    }

    // original car
    int x = S->car.x;
    int y = S->car.y;
    int d = S->car.direction;

    int hx, hy;
    int nd = (d + 2) & 3;

    if (d == UP){
        hx = x + 2;
        hy = y + 1;
    } else if (d == RIGHT){
        hx = x + 1;
        hy = y - 2;
    } else if (d == DOWN){
        hx = x - 2;
        hy = y - 1;
    } else { // LEFT
        hx = x - 1;
        hy = y + 2;
    }

    int c1x, c1y, c2x, c2y;
    if (nd == UP){
        c1x = hx;   c1y = hy;
        c2x = hx;   c2y = hy+1;
    } else if (nd == RIGHT){
        c1x = hx;   c1y = hy;
        c2x = hx+1; c2y = hy;
    } else if (nd == DOWN){
        c1x = hx;   c1y = hy;
        c2x = hx;   c2y = hy-1;
    } else {
        c1x = hx;   c1y = hy;
        c2x = hx-1; c2y = hy;
    }

    if (in_bounds(S,c1x,c1y) && in_bounds(S,c2x,c2y)){
        S->car.x = hx;
        S->car.y = hy;
        S->car.direction = nd;
        return 1;
    }
    return 0;
}

bool touch_target(const State *S, int tx, int ty, int x, int y, int dir){
    // 1×1
    if (S->car_h == 1 && S->car_w == 1)
        return (tx == x && ty == y);

    // 2×2
    if (S->car_h == 2 && S->car_w == 2)
        return (tx == x && ty == y) ||
               (tx == x+1 && ty == y) ||
               (tx == x && ty == y+1) ||
               (tx == x+1 && ty == y+1);

    // 3×2
    if (dir == UP)
        return (tx == x && (ty == y || ty == y+1));
    else if (dir == RIGHT)
        return ((tx == x || tx == x+1) && ty == y);
    else if (dir == DOWN)
        return (tx == x && (ty == y || ty == y-1));
    else // LEFT
        return ((tx == x || tx == x-1) && ty == y);
}