#include "headers/user.h"
#include "libc/headers/stdio.h"

void main(void) {
    while(1) {
    prompt:
        printf("> ");
        char cmdline[128];
        for (int i = 0;; i++) {
            char ch = getc();
            putc(ch);
            if (i == sizeof(cmdline) - 1) {
                printf("Command too long!\n");
                goto prompt;
            } else if (ch == '\r') {
                printf("\n");
                cmdline[i] = '\0';
                break;
            } else {
                cmdline[i] = ch;
            }
        }

        if (strcmp(cmdline, "hello") == 0) {
            printf("Hello world from shell!\n");
        } if (strcmp(cmdline, "exit") == 0) {
            exit();
        } else {
            printf("Unknown command: %s\n", cmdline);
        }
    }
}