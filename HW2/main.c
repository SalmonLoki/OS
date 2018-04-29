#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>

#include <dirent.h>
#include <unistd.h>
#include <wait.h>

const char PARAM_EXEC[] = "-exec";
const char PARAM_INUM[] = "-inum";
const char PARAM_NAME[] = "-name";
const char PARAM_NLINKS[] = "-nlinks";
const char PARAM_SIZE[] = "-size";
char DEFAULT_PATH[] = ".";

enum comp_rule {CR_EQ, CR_GT, CR_LS};

struct sizereq {
    enum comp_rule rule;
    off_t size;
};

struct { //треб инфа о файле
    int req_inum, req_name, req_nlinks, req_size;
    nlink_t nlinks;
    ino_t inum;
    char* name;
    struct sizereq size;
} filereq;

char* exec = "echo";

int check_size(off_t size, struct sizereq* rule) {//подходит ли размер под правило
    switch (rule -> rule) {
        case CR_EQ: return size == rule -> size;
        case CR_GT: return size >= rule -> size;
        case CR_LS: return size <= rule -> size;
    }
}

void procd(char* name) {//рекурсивно пройти директорию
    DIR* dir = opendir(name);
    if (dir == NULL) {
        //fprintf(stderr, "Dir not found\n");
        return;
    }
    //fprintf(stderr, "Dir op\n");
    struct dirent* entry; //элемент содерж.директ.

    while ((entry = readdir(dir)) != NULL) {
        int a = strlen(name) + strlen(entry -> d_name) + 2; //размер полного имени файла
        char b[a];
        sprintf(b, "%s/%s", name, entry -> d_name);
        if (entry -> d_type == DT_DIR) {
            if (strcmp(entry -> d_name, ".") == 0 || strcmp(entry -> d_name, "..") == 0) continue;//текущ.дир. или род.
            procd(b);
        } else {//не дир
            struct stat f_stat;
            stat(b, &f_stat);//получим сатистику fstat файла b
            int f = 1;//фильтр
            f = f && ((!filereq.req_inum) || f_stat.st_ino == filereq.inum);//номер
            f = f && ((!filereq.req_name) || strcmp(entry -> d_name, filereq.name) == 0);//имя
            f = f && ((!filereq.req_nlinks) || f_stat.st_nlink == filereq.nlinks);//кол-во ссылок на файл
            f = f && ((!filereq.req_size) || check_size(f_stat.st_size, &filereq.size));//размер
            if (f) {
                char* args[3];
                args[0] = exec;
                args[1] = b;
                args[2] = NULL;
                if (fork() == 0) {
                    execvp(args[0], args);
                } else {
                    int d;
                    waitpid(-1, &d, 0);
                }
            }
        }
    }
}

int main(int argc, char** argv) {
    int args_proc = 0;
    args_proc++;
    while (args_proc < argc - 1) {
        if (strcmp(argv[args_proc], PARAM_EXEC) == 0) {
            args_proc++;
            exec = argv[args_proc];
            args_proc++;
            continue;
        }
        if (strcmp(argv[args_proc], PARAM_INUM) == 0) {
            args_proc++;
            filereq.req_inum = 1;//значит чекекру нужено пров.inum
            filereq.inum = (ino_t) atoll(argv[args_proc]);
            args_proc++;
            continue;
        }
        if (strcmp(argv[args_proc], PARAM_NAME) == 0) {
            args_proc++;
            filereq.req_name = 1;
            filereq.name = argv[args_proc];
            args_proc++;
            continue;
        }
        if (strcmp(argv[args_proc], PARAM_NLINKS) == 0) {
            args_proc++;
            filereq.req_nlinks = 1;
            filereq.nlinks = (nlink_t) atoll(argv[args_proc]);
            args_proc++;
            continue;
        }
        if (strcmp(argv[args_proc], PARAM_SIZE) == 0) {
            args_proc++;
            filereq.req_size = 1;
            char sign;
            sscanf(argv[args_proc], "%c%i", &sign, &filereq.size.size);
            switch (sign) {
                case '-': filereq.size.rule = CR_LS; break;
                case '+': filereq.size.rule = CR_GT; break;
                case '=': filereq.size.rule = CR_EQ; break;
                default:
                    fprintf(stderr, "Bad size argument, (+/-/=)NUMBER needed");
                    return 1;
            }
            args_proc++;
            continue;
        }
        //if no option found, but still > 1 arguments left
        fprintf(stderr,
                "Bad arguments provided, usage:\n"
                        "find [OPTIONS] [DIRECTORY]\n"
                        "Valid options are:\n"
                        "  -exec PROGRAM -- execute PROGRAM for eny file found\n"
                        "  -inum NUMBER -- search only file with inode number NUMBER\n"
                        "  -name NAME -- search only for files named NAME\n"
                        "  -nlinks NUMBER -- search for files with NUMBER symlinks\n"
                        "  -size SIZE -- search for files with proper SIZE as\n"
                        "    +NUMBER -- at least NUMBER bytes\n"
                        "    -NUMBER -- at most NUMBER bytes\n"
                        "    =NUMBER -- exact NUMBER bytes\n"
        );
        return 1;
    }
    char* path;
    if (args_proc == argc) {
        path = DEFAULT_PATH;
    } else {
        path = argv[args_proc];
    }
    procd(path);
    return 0;
}
