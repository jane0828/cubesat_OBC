#include "common.h"
#include "functions.h"


int send_tmlr(int sock) {
    struct can_frame TMLR;
    TMLR.can_id = CMDTMLR_ID;
    TMLR.can_dlc = 1;
    TMLR.data[0] = 0x00;


    if (write(sock, &TMLR, sizeof(TMLR)) != sizeof(TMLR)) {
        perror("send_tmlr_command");
    } else {
        printf("TMLR 전송 완료 (ID=0x%03X)\n", CMDTMLR_ID);
    }
    

    return 0;
}
