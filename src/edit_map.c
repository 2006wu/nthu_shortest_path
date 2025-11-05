// src/edit.c
#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <string.h>
#include "world.h"
#include "render.h"

static struct termios oldt, newt;
#define EDIT_SAVE_DIR "/Users/2006wu/Desktop/cool_proj/shortest_path_2026/map/target"

static void enter_raw(void){
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    newt.c_cc[VMIN]  = 1;
    newt.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
}

static void leave_raw(void){
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
}

int save_map_to_file(const State *S, const char *fname){
    char full[10000];

    // 你自己的路徑
    const char *base = "/Users/2006wu/Desktop/cool_proj/shortest_path_2026/map/target";

    // base + "/" + fname
    snprintf(full, sizeof(full), "%s/%s", base, fname);

    FILE *fp = fopen(full, "w");
    if (!fp) return -1;

    for (int i=0; i<S->rows; i++){
        for (int j=0; j<S->cols; j++){
            int v = S->maze[i][j];
            if (v == 9) v = 1;   // 把 cursor 還原成一般路
            fputc('0' + v, fp);
        }
        fputc('\n', fp);
    }
    fclose(fp);
    return 0;
}

void edit_map(State *S){
    int cx = 0, cy = 0;      // cursor
    enter_raw();
    for(;;){
        int bak = S->maze[cx][cy];
        S->maze[cx][cy] = CURSOR;     // 9 = cursor
        render_map(S);
        S->maze[cx][cy] = bak;

        char c;
        if (read(STDIN_FILENO, &c, 1) != 1) break;

        if (c == 'q') break;

        // move cursor
        if (c == 'w' && cx > 0)               cx--;
        else if (c == 's' && cx < S->rows-1)  cx++;
        else if (c == 'a' && cy > 0)          cy--;
        else if (c == 'd' && cy < S->cols-1)  cy++;

        // edit
        else if (c == '0') S->maze[cx][cy] = 0;   // wall
        else if (c == '1') S->maze[cx][cy] = 1;   // path
        else if (c == '2') S->maze[cx][cy] = 2;   // start
        else if (c == '3') S->maze[cx][cy] = 3;   // target
        else if (c == '5') S->maze[cx][cy] = 5;   // reached / special

        // 存檔
        else if (c == 'p') {
            char name[256];
            printf("save as (filename only): ");
            fflush(stdout);
            if (fgets(name, sizeof(name), stdin)) {
                name[strcspn(name, "\n")] = 0;

                char fullpath[512];
                snprintf(fullpath, sizeof(fullpath), "%s/%s", EDIT_SAVE_DIR, name);

                FILE *fp = fopen(fullpath, "w");
                if (!fp) {
                    perror("fopen");
                } else {
                    for (int i=0;i<S->rows;i++){
                        for (int j=0;j<S->cols;j++){
                            fprintf(fp, "%d", S->maze[i][j]);
                        }
                        fprintf(fp, "\n");
                    }
                    fclose(fp);
                    printf("saved to %s\n", fullpath);
                }
            }
        }
    }
    leave_raw();
}
