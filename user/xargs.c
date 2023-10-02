#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/param.h"


int getcmd(char *tmp) {
    int size = sizeof(tmp);

    memset(tmp, 0, size);
    gets(tmp, size);

    if (tmp[0] == 0) {
        return -1;
    }
    return 0;
}

char whitespace[] = " \t\r\n\v";

int getToken(char **line, char *lineEnd, char **tokenStart, char **tokenEnd) {
    char *currentChar;
    int ret;

    currentChar = *line;
    while (currentChar < lineEnd && strchr(whitespace, *currentChar))
        currentChar++;
    if (tokenStart) {
        *tokenStart = currentChar;
    }

    ret = *currentChar;
    if (*currentChar != 0) {
        ret = 'x';
        while (currentChar < lineEnd && !strchr(whitespace, *currentChar))
            currentChar++;
    }

    if (tokenEnd) {
        *tokenEnd = currentChar;
    }

    while (currentChar < lineEnd && strchr(whitespace, *currentChar)) {
        currentChar++;
    }

    *line = currentChar;
    return ret;
}

int main(int argc, char *argv[]) {
    char *xargs[MAXARG];
    for (int i = 1; i < argc; i++) {
        xargs[i - 1] = argv[i];
    }

    static char tmp[MAXARG][100];
    char *tokenStart, *tokenEnd;
    int j = argc - 1;
    int i = 0;
    while (getcmd(tmp[i]) >= 0) {
        char *line = tmp[i];
        char *lineEnd = line + strlen(line);

        while (getToken(&line, lineEnd, &tokenStart, &tokenEnd) != 0) {
            xargs[j] = tokenStart;
            *tokenEnd = 0;
            j++;
            i++;
        }

    }

    int pid = fork();
    if (pid == 0) {
        exec(xargs[0], xargs);
    }

    wait(0);
    exit(0);
}