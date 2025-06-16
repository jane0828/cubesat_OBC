#include "common.h"
#include "functions.h"


int send_led_echo(int sock) {
    struct can_frame echo;
    echo.can_id = CMD_LEDECHO_ID;
    echo.can_dlc = 1;
    echo.data[0] = 135;

    write(sock, &echo, sizeof(echo));

    return 0;
}

