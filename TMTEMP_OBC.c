#include "common.h"
#include "functions.h"


int send_tmtemp(int sock) {
    struct can_frame TMTEMP;
    TMTEMP.can_id = CMD_TMTEMP_ID;
    TMTEMP.can_dlc = 1;
    TMTEMP.data[0] = 0x00;


    if (write(sock, &TMTEMP, sizeof(TMTEMP)) != sizeof(TMTEMP)) {
        perror("send_tmlignt_command");
    } else {
        printf("TMLIGHT 전송 완료 (ID=0x%03X)\n", CMD_TMTEMP_ID);
    }
    

    return 0;
}