#include "common.h"
#include "functions.h"

int receive_ack(int sock) {
    struct can_frame ack;

    while (1) {
        if (read(sock, &ack, sizeof(ack)) <= 0) continue;

        // 내 ID로 온 패킷인가?
        if ((ack.can_id & 0x7FF) != FROM_CAM_ID) continue;

        // ACK인지 아닌지 구분
        if (ack.can_id) {
            if (ack.can_dlc == 2) {
                switch (ack.data[0]) {
                    case CMDPIC_ID:
                        if (ack.data[1] == 0x01) return 1;
                        else if (ack.data[1] == 0x02) return 2;
                        else if (ack.data[1] == 0x03) return 3;
                        else if (ack.data[1] == 0x04) return 4;
                        else return 0;    

                        break;

                        
                    case CMDVID_ID:
                        if (ack.data[1] == 0x01) return 1;
                        else if (ack.data[1] == 0x02) return 2;
                        else return 0;

                        break;


                    case CMDRESET_ID:
                        if (ack.data[1] == 0x01) return 1;
                        else if (ack.data[1] == 0x02) return 2;
                        else return 0;
                         
                        break;


                    case CMDECHO_ID:
                        if (ack.data[1] == 0x01) return 1;
                        else if (ack.data[1] == 0x02) return 2;
                        else return 0;
                         
                        break;


                    default:
                        return 0;
                        break;
                }
            }

        } else {
            // ACK가 아닌 일반 메시지 처리
            printf("ECHO 수신됨: can_id=0x%03X, dlc=%d\n",
                   ack.can_id, ack.can_dlc);
            return 0;
            // 필요하면 여기에 일반 명령 처리 로직 추가
        }
    }
}
