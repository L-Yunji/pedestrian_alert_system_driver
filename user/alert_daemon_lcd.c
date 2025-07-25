// alert_daemon_lcd.c
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <poll.h>

#define DEVICE_PATH "/dev/lcd_notify"

int main() {
    int fd = open(DEVICE_PATH, O_RDONLY);
    if (fd < 0) {
        perror("Failed to open device");
        return 1;
    }

    printf("Waiting for LCD trigger...\n");

    struct pollfd pfd = {
        .fd = fd,
        .events = POLLIN
    };

    while (1) {
        int ret = poll(&pfd, 1, -1);
        if (ret < 0) {
            perror("poll error");
            break;
        }

        if (pfd.revents & POLLIN) {
            printf("LCD Trigger received! Perform LCD operation here.\n");

            // 예: LCD 이미지 출력 스크립트 호출
            // system("./display_image.sh");

            // 입력 스트림 읽어서 플래그 초기화
            char dummy;
            read(fd, &dummy, 1);
        }
    }

    close(fd);
    return 0;
}
