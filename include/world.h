#pragma once
#include <stdbool.h>

enum { MAX_ROWS=30, MAX_COLS=30, MAX_SIZE=900 };
enum { WALL=0, PATH=1, START=2, TARGET=3, PASS=4, REACH=5, BLOCKED=6, RE_PASS=7, MINI_PASS=8, CURSOR=9 };
enum { UP=0, RIGHT=1, DOWN=2, LEFT=3 };

typedef struct { 
    int x, y, direction; 
} Car;

typedef struct { 
    int x, y; bool visited; 
} Target;

#define MAX_LOG_STEPS 10000

typedef struct {
    int x, y, d;
} Step;

typedef struct {
    Step steps[MAX_LOG_STEPS];
    int count;
}StepLog;

typedef struct {
    int rows, cols;
    int maze[MAX_ROWS][MAX_COLS];
    Car car;
    Target targets[30];
    int target_cnt;

    // 路徑輸出：每步 (x,y,dir)
    int result_path[MAX_SIZE][3];
    int path_step;

    StepLog runlog;

    int car_h, car_w;
} State;

int replay_cnt;