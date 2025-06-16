#include "common.h"
#include "functions.h"

void print_menu() {
    printf("\n명령을 선택하세요:\n");
    printf("1: CMDHEL(카메라 설정 명령)\n");
    printf("2: CMDPIC(카메라 사진 촬영 및 전송 명령)\n");
    printf("3: CMDVID(카메라 비디오 촬영 및 전송 명령)\n");
    printf("4: CMDRESET(제로 재부팅 명령)\n");
    printf("5: CMDECHO(카메라 에코)\n");
    printf("6: TMSR(간단한 카메라 상태 TM 요청)\n");
    printf("7: TMLR(카메라 전체 상태 TM 요청)\n");
    printf("8: CMDRSVPIC(카메라 사진촬영 예약 명령)\n");
    printf("9: CMDLEDPWR(LED ON/OFF 명령)\n");
    printf("10: TMLIGNT(광 센서값 요청청)\n");
    printf("11: TMTEMT(온도 센서 값 요청)\n");
    printf("12: CMDLEDECHO(LED 에코)\n");
    
    printf("입력 > ");
}

int setup_can_socket(const char *ifname) {
    int s;
    struct sockaddr_can addr;
    struct ifreq ifr;

    s = socket(PF_CAN, SOCK_RAW, CAN_RAW);
    if (s < 0) { perror("socket"); exit(1); }

    strcpy(ifr.ifr_name, ifname);
    ioctl(s, SIOCGIFINDEX, &ifr);

    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;

    if (bind(s, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind"); exit(1);
    }

    return s;
}


// int receive_ack(int sock) {
//     struct can_frame ack;
//     while (1) {
//         if (read(sock, &ack, sizeof(ack)) <=0) continue;

//         if ((ack.can_id & 0x7FF) != (FROM_CAM_ID | FLAG_ACK)) continue;

//         if (ack.can_dlc == 2 && ack.data[0] == CMDPIC_ID) {
//             printf("CMDPIC 전송 성공!\n");
//         }

//         else if (ack.can_dlc == 2 && ack.data[0] == CMDVID_ID) {
//             printf("CMDVID 전송 성공!\n");
//         }

//         else if (ack.can_dlc == 2 && ack.data[0] == CMDRESET_ID) {
//             printf("CMDRESET 전송 성공!\n ACK 돌아옴옴.\n");
//         }

//         else if (ack.can_dlc == 2 && ack.data[0] == CMDECHO_ID) {
//             printf("CMDECHO 전송 성공!\n ECHO 돌아옴.\n");
//         }
//     }
// 
// }
