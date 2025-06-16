#include "common.h"
#include "functions.h"


int send_led_pwr(int sock, const uint8_t data[8]) {
    struct can_frame LEDPWR;
    LEDPWR.can_id = CMDLEDPWR_ID;
    LEDPWR.can_dlc = 1;
     memcpy(LEDPWR.data, data, 1);

    if (write(sock, &LEDPWR, sizeof(LEDPWR)) != sizeof(LEDPWR)) {
        perror("send_led_pwr_command");
    } else {
        printf("LEDPWR 전송 완료 (ID=0x%03X)\n", CMDLEDPWR_ID);
    }
    

    return 0;
}