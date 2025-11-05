#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <dirent.h>
#include <sys/stat.h>
#include <limits.h>

// choose file_path
#include <dirent.h>
#include <sys/stat.h>
#include <limits.h>

static int is_regular_file(const char *path){
    struct stat st;
    return (stat(path, &st) == 0) && S_ISREG(st.st_mode);
}

static int list_files(const char *dir, char paths[][PATH_MAX], int maxn){
    DIR *dp = opendir(dir);
    if (!dp) return -1;
    int n = 0;
    struct dirent *ent;
    while ((ent = readdir(dp)) && n < maxn){
        if (ent->d_name[0] == '.') continue; // 跳過隱藏檔、. ..
        char full[PATH_MAX];
        snprintf(full, sizeof(full), "%s/%s", dir, ent->d_name);
        if (is_regular_file(full)){
            strncpy(paths[n], full, PATH_MAX-1);
            paths[n][PATH_MAX-1] = '\0';
            n++;
        }
    }
    closedir(dp);
    return n;
}

static int cmp_str(const void *a, const void *b){
    const char *sa = *(const char * const *)a;
    const char *sb = *(const char * const *)b;
    return strcmp(sa, sb);
}

static int choose_file_from_dir(const char *dir, char out_path[PATH_MAX]){
    enum { MAXN = 512 };
    char paths[MAXN][PATH_MAX];
    int n = list_files(dir, paths, MAXN);
    if (n <= 0){
        fprintf(stderr, "目錄無檔或無法讀取：%s\n", dir);
        return -1;
    }

    // 排序（以指標陣列輔助）
    const char *pp[MAXN];
    for (int i=0;i<n;i++) pp[i] = paths[i];
    qsort(pp, n, sizeof(pp[0]), cmp_str);

    // 顯示選單
    printf("從資料夾選擇檔案：%s\n", dir);
    for (int i=0;i<n;i++){
        printf("%3d) %s\n", i+1, pp[i]);
    }
    printf("輸入號碼選擇，或 0 取消： ");

    // 讀取選擇
    char buf[64];
    if (!fgets(buf, sizeof(buf), stdin)) return -1;
    int k = atoi(buf);
    if (k <= 0 || k > n) return -1;

    strncpy(out_path, pp[k-1], PATH_MAX-1);
    out_path[PATH_MAX-1] = '\0';
    return 0;
}

// settings
#define MAX_ROWS 30
#define MAX_COLS 30
#define MAX_SIZE 900

#define WALL 0
#define PATH 1
#define START 2
#define TARGET 3

#define UP 0
#define RIGHT 1
#define DOWN 2
#define LEFT 3

int cols = 0, rows = 0;
int maze[MAX_ROWS][MAX_COLS];

typedef struct Target{
    int x;
    int y;
    bool visited;
}Target;

typedef struct Car{
    int x;
    int y;
    int direction;
}Car;

Car car;
Target target[30];
int target_cnt = 0;
int start_x, start_y;

int is_within_bounds(int x, int y){
    return x>=0 && x < rows && y >= 0 && y < cols;
}

void load_map_and_init_setting(const char *map_file){
    FILE *file = fopen(map_file, "r");
    if (!file) {
        printf("Unable to open file.\n");
        exit(1);
    }

    rows = 0;
    char line[MAX_SIZE];
    while (fgets(line, sizeof(line), file)) {
        cols = 0;
        for (int i = 0; line[i] != '\0' && line[i] != '\n'; i++) {
            maze[rows][cols++] = line[i] - '0';
        }
        rows++;
    }
    for (int i=0;i<rows;i++){
        for (int j=0;j<cols;j++){
            if (maze[i][j] == 3){
                target[target_cnt].x = i;
                target[target_cnt].y = j;
                target_cnt++;
            }
            else if (maze[i][j] == 2){
                if (maze[i+1][j] == 2 && maze[i][j+1] == 2 && maze[i][j+2] == 2){
                    car.x = i;
                    car.y = j+2;
                    car.direction = 1;
                }
                else if (maze[i+1][j] == 2 && maze[i+2][j] == 2 && maze[i][j+1] == 2){
                    car.x = i;
                    car.y = j;
                    car.direction = 0;
                }
            }
        }
    }
    fclose(file);
}

void print_map(int target_count){
    system("clear");
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            bool is_car_part = false;
            bool is_car_head = false;

            // 判斷是否為車輛的部分
            if (car.direction == UP) {
                is_car_head = (i == car.x && j == car.y); // 車頭
                is_car_part = (i >= car.x && i < car.x + 3 && j >= car.y && j < car.y + 2);
            } else if (car.direction == LEFT) {
                is_car_head = (i == car.x && j == car.y); // 車頭
                is_car_part = (i <= car.x && i > car.x - 2 && j >= car.y && j < car.y + 3);
            } else if (car.direction == DOWN) {
                is_car_head = (i == car.x && j == car.y); // 車頭
                is_car_part = (i <= car.x && i > car.x - 3 && j <= car.y && j > car.y - 2);
            } else if (car.direction == RIGHT) {
                is_car_head = (i == car.x && j == car.y); // 車頭
                is_car_part = (i >= car.x && i < car.x + 2 && j <= car.y && j > car.y - 3);
            }

            // 顯示車頭位置
            if (is_car_head) {
                printf("🫥");
            }
            // 顯示車身
            else if (is_car_part) {
                printf("🛹");
            }
            // 顯示其他地圖元素
            else if (maze[i][j] == WALL) {
                printf("⬛");
            } else if (maze[i][j] == PATH) {
                printf("⬜");
            } else if (maze[i][j] == START) {
                printf("🟩");
            } else if (maze[i][j] == TARGET) {
                printf("🟥");
            } else if (maze[i][j] == 4) {
                printf("🟦");
            } else if (maze[i][j] == 5) {
                printf("🟧");
            }
        }
        printf("\n");
    }
    usleep(250000); // 延遲 0.25 秒
}

// logic functions
int can_move(int x, int y, int dir){
    int check_x, check_y;
    if (dir == UP) check_y = y++;
    if (dir == RIGHT) check_x = x++;
    if (dir == DOWN) check_y = y--;
    if (dir == LEFT) check_x = x--;
    if (is_within_bounds(check_x, check_y) && maze[check_x][check_y] != WALL){
        return 1;
    }
    return 0;
}

int can_turn(int x, int y, int dir, int *new_x, int *new_y){
    int check_x, check_y;
    // opposite direction
    // turn right
    // turn left
    return true;
}

bool touch_target(int target_x, int target_y, int x, int y, int dir){
    if (dir == UP){
        return (x == target_x && y == target_y) || (x == target_x && y+1 == target_y) ||
        (x+1 == target_x && y == target_y) || (x+1 == target_x && y+1 == target_y) ||
        (x+2 == target_x && y == target_y) || (x+2 == target_x && y+1 == target_y);
    }
    else if (dir == RIGHT){
        return (x == target_x && y == target_y) || (x+1 == target_x && y == target_y) ||
        (x == target_x && y-1 == target_y) || (x+1 == target_x && y-1 == target_y) ||
        (x == target_x && y-2 == target_y) || (x+1 == target_x && y-2 == target_y);
    }
    else if (dir == DOWN){
        return (x == target_x && y == target_y) || (x == target_x && y-1 == target_y) ||
        (x-1 == target_x && y == target_y) || (x-1 == target_x && y-1 == target_y) ||
        (x-2 == target_x && y == target_y) || (x-2 == target_x && y-1 == target_y);
    }
    else if (dir == LEFT){
        return (x == target_x && y == target_y) || (x-1 == target_x && y == target_y) ||
        (x == target_x && y+1 == target_y) || (x-1 == target_x && y+1 == target_y) ||
        (x == target_x && y+2 == target_y) || (x-1 == target_x && y+2 == target_y);
    }
    return false;
}

int result_path[MAX_SIZE][3];
int path_step = 10000;

void print_path(int count){
    for (int i=0;i<path_step;i++){
        car.x = result_path[i][0];
        car.y = result_path[i][1];
        car.direction = result_path[i][2];
        maze[car.x][car.y] = 4;
        print_map(count);
        maze[car.x][car.y] = PATH;
    }
}

int visited[MAX_ROWS][MAX_COLS];
int temp_path[MAX_SIZE][3];
int temp_step = 0;

int algorithm_dijkstra(){
    int count = target_cnt;

    return 0;
}

int bfs(){
    return 0;
}

int main(int argc, char **argv){
    const char *default_dir =
        "/Users/2006wu/Desktop/cool_proj/shorteset_path_2025/map";  // 改成你的資料夾
    char chosen[PATH_MAX];

    const char *map_path = NULL;
    if (argc >= 2) {
        map_path = argv[1];  // 直接用參數
    } else {
        if (choose_file_from_dir(default_dir, chosen) != 0) {
            fprintf(stderr, "未選擇檔案或失敗。\n");
            return 1;
        }
        map_path = chosen;  // 用互動選擇
    }

    target_cnt = 0;
    rows = cols = 0;
    memset(maze, 0, sizeof(maze));

    load_map_and_init_setting(map_path);
    print_map(target_cnt);

    if (target_cnt == 0) bfs();
    else    algorithm_dijkstra();
    return 0;
}