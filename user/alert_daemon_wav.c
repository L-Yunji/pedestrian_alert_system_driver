#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <poll.h>
#include <string.h>

#define DEVICE_PATH "/dev/wav_notify"

int main() {
    int fd = open(DEVICE_PATH, O_RDWR);
    if (fd < 0) {
        perror("open");
        return 1;
    }

    struct pollfd pfd = { .fd = fd, .events = POLLIN };

    printf("Waiting for sound trigger...\n");

    while (1) {
        int ret = poll(&pfd, 1, -1);
        if (ret < 0) {
            perror("poll");
            break;
        }

        if (pfd.revents & POLLIN) {
            char filename[64] = {0};
            read(fd, filename, sizeof(filename));
            printf("Trigger received. Playing: %s\n", filename);

            char cmd[128];
            snprintf(cmd, sizeof(cmd), "aplay -D plughw:1,0 %s", filename);
            system(cmd);
        }
    }

    close(fd);
    return 0;
}
