//#include "common.h"
//#include "functions.h"

// int receive_ack(int sock) {
//     struct can_frame ack;
//     while (1) {
//         if (read(sock, &ack, sizeof(ack)) <= 0) continue;

//         // ID 확인: FROM_CAM_ID이고, FLAG_ACK가 붙어야 함
//         if (((ack.can_id & 0x7FF) != FROM_CAM_ID) || !(ack.can_id & FLAG_ACK)) continue;

//         if (ack.can_dlc == 2) {
//             switch (ack.data[0]) {
//                 case CMDPIC_ID:
//                     printf("CMDPIC 전송 성공!\n");
//                     break;
//                 case CMDVID_ID:
//                     printf("CMDVID 전송 성공!\n");
//                     break;
//                 case CMDRESET_ID:
//                     printf("CMDRESET 전송 성공!\n");
//                     break;
//                 case CMDECHO_ID:
//                     printf("CMDECHO 전송 성공!\n");
//                     break;
//                 default:
//                     printf("알 수 없는 ACK 수신: 0x%02X\n", ack.data[0]);
//                     break;
//             }
//         }
//     }
// }


/*
int receive_ack(int sock) {
    struct can_frame ack;

    while (1) {
        if (read(sock, &ack, sizeof(ack)) <= 0) continue;

        // 내 ID로 온 패킷인가?
        if ((ack.can_id & 0x7FF) != FROM_CAM_ID) continue;

        // ACK인지 아닌지 구분
        if (ack.can_id & FLAG_ACK) {
            if (ack.can_dlc == 2) {
                switch (ack.data[0]) {
                    case CMDPIC_ID:
                        printf("CMDPIC 전송 성공!\n");
                        break;
                    case CMDVID_ID:
                        printf("CMDVID 전송 성공!\n");
                        break;
                    case CMDRESET_ID:
                        printf("CMDRESET 전송 성공!\n");
                        break;
                    case CMDECHO_ID:
                        printf("CMDECHO 전송 성공!\n");
                        break;
                    default:
                        printf("알 수 없는 ACK 수신: 0x%02X\n", ack.data[0]);
                        break;
                }
            }
        } else {
            // ACK가 아닌 일반 메시지 처리
            printf("ECHO 수신됨: can_id=0x%03X, dlc=%d\n",
                   ack.can_id, ack.can_dlc);
            // 필요하면 여기에 일반 명령 처리 로직 추가
        }
    }
}
*/


#include <stdio.h>
#include <linux/can.h>
#include <unistd.h>
#include "common.h"

int receive_ack(int sock) {
    struct can_frame frame;
    int nbytes = read(sock, &frame, sizeof(struct can_frame));
    
    if (nbytes < 0) {
        perror("CAN read failed");
        return -1;
    }

    switch (frame.can_id) {
        case 0x100:  //카메라 TC 응답
            if (frame.data[0] == CMDHEL_ID) {
                printf("카메라 설정 성공\n");
                return 0;
            } 
            else if (frame.data[0] == CMDPIC_ID) {
                printf("사진촬영 명령 성공\n");
                return 0;
            } 
            else if (frame.data[0] == CMDVID_ID) {
                printf("사진촬영 명령 성공\n");
                return 0;
            } 
            else if (frame.data[0] == CMDRESET_ID) {
                printf("리셋 명령 성공\n");
                return 0;
            } 
            else if (frame.data[0] == CMDECHO_ID) {
                printf("에코 성공\n");
                return 0;
            }
            else if (frame.data[0] == CMDRSV_ID) {
                printf("예약 명령 성공\n");
                return 0;
            }
            break;
        case 0x200:  //LED TC 응답
            if (frame.data[0] == CMDLEDPWR_ID) {
                printf("LED ON/OFF 명령 성공\n");
                return 0;
            }
            else if (frame.data[0] == CMD_LEDECHO_ID) {
                printf("LED 에코 성공\n");
                return 0;
            } 
            break;
        case 0x137: //TMSR 응답
            break;
        case 0x138: //TMLR 응답
            break;
        case 0x251: //TMLIGHT 응답
            break;
        case 0x252: //TMTEMP 응답
            break;


        default:
            printf("⚠️ 알 수 없는 메시지 ID: 0x%X\n", frame.can_id);
            return -1;
    }
}