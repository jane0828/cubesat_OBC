#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <stdint.h>
#include <time.h>

#define CMD_ID 0x100
#define DATA_ID_BASE 0x200
#define DATA_ID_END  0x2FF
#define END_FRAME_ID 0x2FF
#define END_FRAME_VAL 0xFF
#define ECHO_ID 0x07
#define STATUS_ID 0x300
#define OUTPUT_FILE "received.jpg"

void setup_can(int *sock, struct sockaddr_can *addr, struct ifreq *ifr) {
    *sock = socket(PF_CAN, SOCK_RAW, CAN_RAW);
    if (*sock < 0) { perror("socket"); exit(1); }

    strcpy(ifr->ifr_name, "can0");
    ioctl(*sock, SIOCGIFINDEX, ifr);
    addr->can_family = AF_CAN;
    addr->can_ifindex = ifr->ifr_ifindex;

    if (bind(*sock, (struct sockaddr *)addr, sizeof(*addr)) < 0) {
        perror("bind"); exit(1);
    }
}

void send_command(int sock, unsigned char code) {
    struct can_frame frame;
    frame.can_id = CMD_ID;
    frame.can_dlc = 1;
    frame.data[0] = code;

    if (write(sock, &frame, sizeof(frame)) != sizeof(frame)) {
        perror("send_command");
    } else {
        printf("명령 전송 완료 (0x%02X)\n", code);
    }
}

void handle_status_frame(struct can_frame *frame) {
    if (frame->can_dlc < 5) return;

    unsigned char code = frame->data[0];
    uint32_t ts = (frame->data[1] << 24) | (frame->data[2] << 16) | (frame->data[3] << 8) | frame->data[4];
    char time_str[64];
    time_t t = ts;
    struct tm *tm_info = localtime(&t);
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", tm_info);

    switch (code) {
        case 0x09:
            printf("[STATUS] 사진 촬영 시작 (%s)\n", time_str);
            break;
        case 0x10:
            printf("[STATUS] 사진 촬영 완료 (%s)\n", time_str);
            break;
        case 0x11:
            printf("[STATUS] 이미지 전송 시작 (%s)\n", time_str);
            break;
        default:
            printf("[STATUS] 알 수 없는 상태 코드 (0x%02X) (%s)\n", code, time_str);
            break;
    }
}

void receive_image(int sock) {
    unsigned char *buffer = NULL;
    size_t buffer_size = 0;
    size_t offset = 0;
    struct can_frame frame;
    struct timeval start_time, end_time;
    int first = 1;

    printf("이미지 수신 대기 중...\n");

    while (1) {
        int nbytes = read(sock, &frame, sizeof(frame));
        if (nbytes <= 0) continue;

        if (frame.can_id == STATUS_ID) {
            handle_status_frame(&frame);
            continue;
        }

        if (first && frame.can_id >= DATA_ID_BASE && frame.can_id <= DATA_ID_END) {
            gettimeofday(&start_time, NULL);
            first = 0;
        }

        if (frame.can_id == END_FRAME_ID && frame.can_dlc == 1 && frame.data[0] == END_FRAME_VAL) {
            printf("전송 종료 프레임 수신\n");
            break;
        }

        if (frame.can_id >= DATA_ID_BASE && frame.can_id <= DATA_ID_END) {
            size_t needed = offset + frame.can_dlc;
            if (needed > buffer_size) {
                buffer_size = needed + 1024;
                buffer = realloc(buffer, buffer_size);
                if (!buffer) {
                    fprintf(stderr, "메모리 재할당 실패\n");
                    return;
                }
            }

            memcpy(buffer + offset, frame.data, frame.can_dlc);
            offset += frame.can_dlc;
        }
    }

    gettimeofday(&end_time, NULL);
    long elapsed_sec = end_time.tv_sec - start_time.tv_sec;
    long elapsed_usec = end_time.tv_usec - start_time.tv_usec;
    double elapsed_ms = elapsed_sec * 1000.0 + elapsed_usec / 1000.0;

    FILE *fp = fopen(OUTPUT_FILE, "wb");
    if (fp) {
        fwrite(buffer, 1, offset, fp);
        fclose(fp);
        printf("이미지 저장 완료: %s (%zu bytes)\n", OUTPUT_FILE, offset);
    } else {
        perror("fopen");
    }

    printf("총 수신 시간: %.2f ms (%.2f 초)\n", elapsed_ms, elapsed_ms / 1000.0);
    free(buffer);
}

void echo_test(int sock) {
    struct can_frame frame;
    send_command(sock, 0x07);
    printf("에코 응답 대기 중...\n");

    fd_set read_fds;
    struct timeval timeout;

    FD_ZERO(&read_fds);
    FD_SET(sock, &read_fds);
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;

    int ret = select(sock + 1, &read_fds, NULL, NULL, &timeout);
    if (ret > 0 && FD_ISSET(sock, &read_fds)) {
        if (read(sock, &frame, sizeof(frame)) > 0) {
            if (frame.can_id == ECHO_ID && frame.data[0] == 0x07) {
                printf("에코 응답 수신됨! (0x%02X)\n", frame.data[0]);
            } else {
                printf("에코 응답이 예상과 다름\n");
            }
        }
    } else {
        printf("에코 응답 없음 (타임아웃)\n");
    }
}

void reboot_with_echo_wait(int sock) {
    send_command(sock, 0x05);
    printf("재부팅 명령 전송 완료. Echo 응답 대기 중 (최대 300초)...\n");

    fd_set read_fds;
    struct timeval timeout;
    struct can_frame frame;

    int total_wait_sec = 300;
    int waited = 0;

    while (waited < total_wait_sec) {
        FD_ZERO(&read_fds);
        FD_SET(sock, &read_fds);
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;

        int ret = select(sock + 1, &read_fds, NULL, NULL, &timeout);
        if (ret > 0 && FD_ISSET(sock, &read_fds)) {
            if (read(sock, &frame, sizeof(frame)) > 0) {
                if (frame.can_id == ECHO_ID && frame.data[0] == 0x07) {
                    printf("\n재부팅 후 Echo 응답 수신됨!\n 재부팅 성공");
                    return;
                }
                if (frame.can_id == STATUS_ID) {
                    handle_status_frame(&frame);
                    continue;
                }
            }
        }

        waited++;
        if (waited % 60 == 0) {
            printf("%d분 경과\n", waited / 60);
        }
        if (waited % 5 == 0) {
            printf(".");
            fflush(stdout);
        }
    }

    printf("\nEcho 응답 없음. 재부팅 실패 또는 통신 문제일 수 있음.\n");
}

void print_menu() {
    printf("\n명령을 선택하세요:\n");
    printf("5: Raspberry Pi Zero 재부팅 요청\n");
    printf("6: 사진 촬영 및 수신\n");
    printf("7: 에코 테스트 (ping)\n");
    printf("0: 종료\n");
    printf("입력 > ");
}

int main() {
    int sock;
    struct sockaddr_can addr;
    struct ifreq ifr;
    int choice;

    setup_can(&sock, &addr, &ifr);

    while (1) {
        print_menu();
        scanf("%d", &choice);

        switch (choice) {
            case 0:
                printf("종료합니다.\n");
                return 0;
            case 5:
                reboot_with_echo_wait(sock);
                break;
            case 6:
                send_command(sock, 0x06);
                receive_image(sock);
                break;
            case 7:
                echo_test(sock);
                break;
            default:
                printf("유효하지 않은 선택입니다.\n");
        }
    }
    return 0;
}
