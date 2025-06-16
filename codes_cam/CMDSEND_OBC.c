#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#include <net/if.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <stdint.h>

#define CMD_ID     0x200
#define DATA_ID    0x300
#define END_ID     0x3FF
#define NAME_ID    0x3FE
#define SAVE_DIR   "/home/doteam/Desktop/Camera_team/received_pics"

int setup_can_socket(const char *ifname) {
    int s;
    struct sockaddr_can addr;
    struct ifreq ifr;

    s = socket(PF_CAN, SOCK_RAW, CAN_RAW);
    strcpy(ifr.ifr_name, ifname);
    ioctl(s, SIOCGIFINDEX, &ifr);
    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;
    bind(s, (struct sockaddr *)&addr, sizeof(addr));
    return s;
}

void send_photo_request(int sock, int year, int month, int day, int hour) {
    struct can_frame frame;
    uint8_t data[5] = {
        0x01,
        (uint8_t)(year - 2000),
        (uint8_t)month,
        (uint8_t)day,
        (uint8_t)hour
    };

    for (int i = 0; i < 5; i++) {
        frame.can_id = CMD_ID + i;
        frame.can_dlc = 1;
        frame.data[0] = data[i];
        write(sock, &frame, sizeof(frame));
    }

    printf("사진 요청 전송 완료: %04d-%02d-%02d %s\n",
           year, month, day, (hour == 24 ? "(전체 시간)" : ""));
}

void receive_photo(int sock) {
    struct can_frame frame;
    char filename[256] = {0}, fullpath[512] = {0};
    int fname_index = 0, receiving_filename = 0;
    FILE *fp = NULL;

    while (1) {
        if (read(sock, &frame, sizeof(frame)) > 0) {
            if (frame.can_id == NAME_ID) {
                if (frame.data[0] == 0x00 && frame.can_dlc == 1 && receiving_filename) {
                    filename[fname_index] = '\0';
                    receiving_filename = 0;

                    if (strlen(filename) < 5 || strchr(filename, '.') == NULL) continue;
                    snprintf(fullpath, sizeof(fullpath), "%s/%s", SAVE_DIR, filename);
                    fp = fopen(fullpath, "wb");
                    if (!fp) { perror("파일 열기 실패"); return; }
                    printf("파일 저장 시작: %s\n", fullpath);
                }
                else if (frame.can_dlc > 0 && !receiving_filename && fname_index == 0) {
                    receiving_filename = 1;
                    memcpy(filename, frame.data, frame.can_dlc);
                    fname_index += frame.can_dlc;
                    filename[fname_index] = '\0';
                }
                else if (receiving_filename) {
                    if (fname_index + frame.can_dlc < sizeof(filename)) {
                        memcpy(filename + fname_index, frame.data, frame.can_dlc);
                        fname_index += frame.can_dlc;
                        filename[fname_index] = '\0';
                    } else {
                        fname_index = 0;
                        receiving_filename = 0;
                    }
                }
            }
            else if ((frame.can_id & 0xF00) == DATA_ID && fp) {
                fwrite(frame.data, 1, frame.can_dlc, fp);
            }
            else if (frame.can_id == END_ID && frame.data[0] == 0xFF) {
                if (fp) {
                    fclose(fp);
                    printf("저장 완료: %s\n", fullpath);
                    fp = NULL;
                }
                fname_index = 0;
                receiving_filename = 0;
                memset(filename, 0, sizeof(filename));
                memset(fullpath, 0, sizeof(fullpath));
            }
        }
    }
}

int main() {
    int year, month, day, hour;
    int sock = setup_can_socket("can0");

    printf("요청할 사진 날짜를 입력하세요.\n");
    printf("연도 (예: 2025): ");
    scanf("%d", &year);
    printf("월 (1~12): ");
    scanf("%d", &month);
    printf("일 (1~31): ");
    scanf("%d", &day);
    printf("시 (0~23, 또는 전체는 24): ");
    scanf("%d", &hour);

    send_photo_request(sock, year, month, day, hour);
    receive_photo(sock);
    close(sock);
    return 0;
}
