#include "common.h"
#include "functions.h"


int send_tmsr(int sock) {
    struct can_frame TMSR;
    TMSR.can_id = CMDTMSR_ID;
    TMSR.can_dlc = 1;
    TMSR.data[0] = 0x00;


    if (write(sock, &TMSR, sizeof(TMSR)) != sizeof(TMSR)) {
        perror("send_tmsr_command");
    } else {
        printf("TMSR 전송 완료 (ID=0x%03X)\n", CMDTMSR_ID);
    }

   

    return 0;
}
