#include "common.h"
#include "functions.h"


int send_tmlight(int sock) {
    struct can_frame TMLIGNT;
    TMLIGNT.can_id = CMD_TMLIGNT_ID;
    TMLIGNT.can_dlc = 1;
    TMLIGNT.data[0] = 0x00;


    if (write(sock, &TMLIGNT, sizeof(TMLIGNT)) != sizeof(TMLIGNT)) {
        perror("send_tmlignt_command");
    } else {
        printf("TMLIGHT 전송 완료 (ID=0x%03X)\n", CMD_TMLIGNT_ID);
    }
    

    return 0;
}