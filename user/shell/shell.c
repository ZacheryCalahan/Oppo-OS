#include "../user.h"
#include "../libc/stdio.h"

#include <stdint.h>

void main(void) {

    // while(1) {
    // prompt:
    //     printf("> ");
    //     char cmdline[128];
    //     for (int i = 0;; i++) { // Read in chars
    //         char ch = getc();
    //         putc(ch);
    //         if (i == sizeof(cmdline) - 1) {
    //             printf("Command too long!\n");
    //             goto prompt;
    //         } else if (ch == '\r') {
    //             printf("\n");
    //             cmdline[i] = '\0';
    //             break;
    //         } else {
    //             cmdline[i] = ch;
    //         }
    //     }

    //     if (strcmp(cmdline, "hello") == 0) {
    //         printf("Hello world from shell!\n");
    //     } else if (strcmp(cmdline, "exit") == 0) {
    //         exit();
    //     } else {
    //         printf("Unknown command: %s\n", cmdline);
    //     }
    // }

    // For sake of user testing, and not messing with the .ld stuffs, I'll be
    // using this for testing and not a shell. Complaints can be submitted to B. Hol.

    int32_t fd = open("hello.txt", 3);
    char text[4096];
    read(fd, text, 4096);
    printf("Userland syscalls!\n\nText for file below\n\n%s\n", text);
    exit();
}

void change_dirs() {

}