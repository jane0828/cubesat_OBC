#include "common.h"
#include "functions.h"
//TMLIGHT 요청
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
//TMTEMP요청
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
//LED 에코 요청
int send_led_echo(int sock) {
    struct can_frame echo;
    echo.can_id = CMD_LEDECHO_ID;
    echo.can_dlc = 1;
    echo.data[0] = 135;

    write(sock, &echo, sizeof(echo));

    return 0;
}
//LEDPWR 명령
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