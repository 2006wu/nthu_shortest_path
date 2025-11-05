#include "stdio.h"
#include "string.h"
#include "limits.h"
#include "stdlib.h"
#include "unistd.h"

#include "world.h"
#include "file_picker.h"
#include "map.h"
#include "motion.h"
#include "algo.h"
#include "render.h"
#include "scan.h"
#include "edit_map.h"

int main(int argc, char **argv){
    State S; memset(&S, 0, sizeof(S));

    /* choose version */
    int version = 0;
    if (argc < 3) {
        printf("Select version: 1) greedy  2) dijkstra-first-unvisited  3) scan-all  4) random : ");
        char buf[32];
        if (!fgets(buf, sizeof(buf), stdin)) return 1;
        version = atoi(buf);
    } else {
        version = atoi(argv[2]);
    }
    if (version < 1 || version > 4) {
        fprintf(stderr, "invalid version\n");
        return 1;
    }

    /* choose folder according to the version */
    const char *dir_target = "/Users/2006wu/Desktop/cool_proj/shortest_path_2026/map/target";
    const char *dir_scan   = "/Users/2006wu/Desktop/cool_proj/shortest_path_2026/map/scan";

    const char *map_path = NULL;
    char chosen[PATH_MAX];

    if (version == 4) {
        /* random map */
        if (load_random_map(&S, 20, 20) < 0) {
            fprintf(stderr, "random map fail\n");
            return 1;
        }
    } else {
        const char *base_dir = (version == 3) ? dir_scan : dir_target;
        if (argc >= 2) {
            map_path = argv[1];
        } else {
            if (choose_file_from_dir(base_dir, chosen) != 0) {
                fprintf(stderr, "choose map fail\n");
                return 1;
            }
            map_path = chosen;
        }
        if (load_map_and_init(&S, map_path) < 0) return 2;
    }

    render_map(&S);

    /* edit map */
    edit_map(&S);   // 不想編輯可以在裡面按 q

    /* ask to save */
    printf("Save edited map? (y/n): ");
    char buf[256];
    if (fgets(buf, sizeof(buf), stdin) && buf[0] == 'y') {
        printf("filename: ");
        if (fgets(buf, sizeof(buf), stdin)) {
            // 去掉換行
            buf[strcspn(buf, "\n")] = 0;
            if (buf[0]) {
                if (save_map_to_file(&S, buf) == 0)
                    printf("saved to %s\n", buf);
                else
                    printf("save failed\n");
            }
        }
    }

    /* car_size */
    printf("Select car size: 1) 3x2  2) 2x2 : ");
    {
        char buf[32];
        if (fgets(buf, sizeof(buf), stdin)) {
            int v = atoi(buf);
            if (v == 2) {
                S.car_h = 2;
                S.car_w = 2;
            } else {
                S.car_h = 3;
                S.car_w = 2;
            }
        } else {
            S.car_h = 3;
            S.car_w = 2;
        }
    }

    S.runlog.count = 0;

    /* choose mode to show */
    int mode = 1;
    if (version == 1 || version == 2) {
        printf("Select mode: 1) normal run  2) step-by-step : ");
        char buf[32];
        if (fgets(buf, sizeof(buf), stdin)) {
            int v = atoi(buf);
            if (v == 2) mode = 2;
        }
    }

    /* run */
    if (version == 1) {
        if (S.target_cnt == 0) {
            fprintf(stderr, "no target for greedy\n");
            return 0;
        }
        greedy_run_all_target(&S, mode);
    } else if (version == 2) {
        if (S.target_cnt == 0) {
            fprintf(stderr, "no target for dijkstra\n");
            return 0;
        }
        run_all_target_first_unvisited(&S, mode);
    } else if (version == 3) {
        run_scan_full(&S);
    } else { // version == 4
        // 隨機圖也來掃一次
        run_scan_full(&S);
    }

    /* rp */
    reset_stdin_block();
    replay_cnt = 0;

    // 在進入 1×1 前重新確認哪些 target 沒被觸碰
    for (int i = 0; i < S.target_cnt; i++) {
        int tx = S.targets[i].x;
        int ty = S.targets[i].y;
        int touched = 0;

        // 如果那個 target 還在地圖上，檢查是否真的被 REACH 覆蓋過
        if (S.maze[tx][ty] == REACH)
            touched = 1;

        // 沒有被 REACH 的 target 就標成未訪問
        if (!touched) {
            S.targets[i].visited = 0;
        }
    }


    while (1) {
        printf("\nAll 3×2 targets finished.\n");
        printf("1. Explore with 1×1 from current car position\n");
        printf("2. Replay previous path\n");
        printf("3. Finish\n");
        printf("Select (1–3): ");

        char buf[16];
        if (!fgets(buf, sizeof(buf), stdin)) {
            clearerr(stdin);
            continue;
        }
        // explore with 1x1
        if (buf[0] == '1') {
            int remaining = 0;
            for (int i = 0; i < S.target_cnt; i++) {
                if (!S.targets[i].visited) remaining++;
            }
            if (remaining == 0) {
                printf("No remaining targets. All already visited.\n");
                continue;
            }

            int old_h = S.car_h, old_w = S.car_w;
            S.car_h = 1; 
            S.car_w = 1;

            for (int i = 0; i < S.rows; i++) {
                for (int j = 0; j < S.cols; j++) {
                    if (S.maze[i][j] == REACH) S.maze[i][j] = PATH;
                }
            }

            printf("Exploring remaining targets with 1×1...\n");
            run_all_target_first_unvisited(&S, 1);

            S.car_h = old_h; S.car_w = old_w;
        }
        // rp
        else if (buf[0] == '2') {
            replay_runlog(&S);
        } 
        // finish
        else if (buf[0] == '3') {
            printf("Finish.\n");
            break;
        } 
        else {
            printf("Invalid selection.\n");
        }
        usleep(1000000);
    }

    return 0;
}