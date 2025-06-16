#include "common.h"
#include "functions.h"


int send_echo(int sock) {
    struct can_frame echo;
    echo.can_id = CMDECHO_ID;
    echo.can_dlc = 1;
    echo.data[0] = 0x08;

    write(sock, &echo, sizeof(echo));

    return 0;
}


// int main() {
//     int sock = setup_can_socket("can0");
//     struct can_frame frame;

//     printf("Echo 응답 수신 대기 중 (ID 0x135)...\n");

//     while (1) {
//         if (read(sock, &frame, sizeof(frame)) > 0) {
//             if (frame.can_id == ECHO_ID && frame.can_dlc >= 1 && frame.data[0] == 0x07) {
//                 printf("Echo 응답 수신 성공! (ID: 0x%03X, data[0]: 0x%02X)\n",
//                        frame.can_id, frame.data[0]);
//                 break;  // 필요 시 break 없애고 계속 수신 가능
//             } else {
//                 printf("다른 메시지 수신 (ID: 0x%03X, data[0]: 0x%02X)\n",
//                        frame.can_id, frame.data[0]);
//             }
//         }
//     }

//     close(sock);
//     return 0;
// }
