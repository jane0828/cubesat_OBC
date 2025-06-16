#include "common.h"
#include "functions.h"

void send_reboot_command(int sock) {
    struct can_frame cmd = { .can_id = CMDRESET_ID, .can_dlc = 1, .data = {0x001} };
    if (write(sock, &cmd, sizeof(cmd)) != sizeof(cmd)) {
        perror("명령 전송 실패");
    } else {
        printf("명령 전송: 0x001\n");
    }
}

// void reboot_with_echo_wait(int sock) {
//     send_command(sock, 0x05);
//     printf("재부팅 명령 전송 완료. Echo 응답 대기 중 (최대 300초)...\n");

//     fd_set read_fds;
//     struct timeval timeout;
//     struct can_frame frame;
//     int waited = 0;

//     while (waited < 300) {
//         FD_ZERO(&read_fds);
//         FD_SET(sock, &read_fds);
//         timeout.tv_sec = 1;
//         timeout.tv_usec = 0;

//         int ret = select(sock + 1, &read_fds, NULL, NULL, &timeout);
//         if (ret > 0 && FD_ISSET(sock, &read_fds)) {
//             if (read(sock, &frame, sizeof(frame)) > 0) {
//                 if (frame.can_id == ECHO_ID && frame.data[0] == 0x07) {
//                     printf("\n재부팅 후 Echo 응답 수신됨! 재부팅 성공\n");
//                     return;
//                 }
//             }
//         }

//         waited++;
//         if (waited % 60 == 0) printf("%d분 경과\n", waited / 60);
//         if (waited % 5 == 0) { printf("."); fflush(stdout); }
//     }

//     printf("\nEcho 응답 없음. 재부팅 실패 또는 통신 문제일 수 있음.\n");
// }

// int setup_can_socket(const char *ifname) {
//     int s;
//     struct sockaddr_can addr;
//     struct ifreq ifr;

//     s = socket(PF_CAN, SOCK_RAW, CAN_RAW);
//     if (s < 0) { perror("socket"); exit(1); }

//     strcpy(ifr.ifr_name, ifname);
//     ioctl(s, SIOCGIFINDEX, &ifr);

//     addr.can_family = AF_CAN;
//     addr.can_ifindex = ifr.ifr_ifindex;

//     if (bind(s, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
//         perror("bind"); exit(1);
//     }
//     return s;
// }

// int main() {
//     int sock = setup_can_socket("can0");
//     reboot_with_echo_wait(sock);
//     close(sock);
//     return 0;
// }
