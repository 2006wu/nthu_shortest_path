#pragma once
#include "world.h"

void replay_runlog(State *S);
void set_stdin_nonblock(void);
void reset_stdin_block(void);

/* 單一 target 的最短路。
   回傳 0 表示找到路，S->result_path 與 S->path_step 會寫好，
   並且 S->car 會被移到該 target 的姿勢。<0 表示到不了。 */
int dijkstra_one_target(State *S, int target_idx);

/* 跑多個 target：
   每次先選還沒visited且絕對距離最近的target，去跑一次 dijkstra
   自動把到不了的列印出來。
   回傳成功到達的target數量。 */
int greedy_run_all_target(State *S, int reply_mode);

int dijkstra_first_unvisited(State *S);
int run_all_target_first_unvisited(State *S, int reply_mode);