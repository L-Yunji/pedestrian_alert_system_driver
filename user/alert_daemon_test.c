#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <poll.h>

#define DEVICE_PATH "/dev/alert_trigger"

int main() {
    int fd = open(DEVICE_PATH, O_RDONLY);
    if (fd < 0) {
        perror("open");
        return 1;
    }

    struct pollfd pfd;
    pfd.fd = fd;
    pfd.events = POLLOUT;

    printf("Waiting for alert...\n");

    while (1) {
        int ret = poll(&pfd, 1, -1);  // timeout: -1 → 무한 대기
        if (ret < 0) {
            perror("poll");
            break;
        }

        if (pfd.revents & POLLOUT) {
            printf("[EVENT] Alert detected! 처리 시작...\n");

            // 커널에서 alert_flag를 다시 0으로 만들기 위해 read 호출
            char dummy;
            read(fd, &dummy, 1);  // 데이터는 안 쓰이지만 flag 리셋 용도
        }
    }

    close(fd);
    return 0;
}
