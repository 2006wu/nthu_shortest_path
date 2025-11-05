#include "file_picker.h"
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <stdlib.h>

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
        if (ent->d_name[0] == '.') continue;
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

int choose_file_from_dir(const char *dir, char out_path[PATH_MAX]){
    enum { MAXN = 512 };
    char paths[MAXN][PATH_MAX];
    int n = list_files(dir, paths, MAXN);
    if (n <= 0){
        fprintf(stderr, "目錄無檔或無法讀取：%s\n", dir);
        return -1;
    }

    const char *pp[MAXN];
    for (int i=0;i<n;i++) pp[i] = paths[i];
    qsort(pp, n, sizeof(pp[0]), cmp_str);

    printf("從資料夾選擇檔案：%s\n", dir);
    for (int i=0;i<n;i++){
        printf("%3d) %s\n", i+1, pp[i]);
    }
    printf("輸入號碼選擇，或 0 取消： ");

    char buf[64];
    if (!fgets(buf, sizeof(buf), stdin)) return -1;
    int k = atoi(buf);
    if (k <= 0 || k > n) return -1;

    strncpy(out_path, pp[k-1], PATH_MAX-1);
    out_path[PATH_MAX-1] = '\0';
    return 0;
}
