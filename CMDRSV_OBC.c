#include "common.h"
#include "functions.h"

void send_rsv_utc(int sock, const uint8_t data[8]) {
    struct can_frame rsv;
    rsv.can_id = CMDRSV_ID;
    rsv.can_dlc = 7;
    memcpy(rsv.data, data, 7);

    if (write(sock, &rsv, sizeof(rsv)) != sizeof(rsv)) {
        perror("send_rsv_utc_command");
    } else {
        printf("UTC 예약명령 전송 완료 (ID=0x%03X)\n", CMDRSV_ID);
    }
}

void send_rsv_rel(int sock, const uint8_t data[8]) {
    struct can_frame rsv;
    rsv.can_id = CMDRSV_ID;
    rsv.can_dlc = 4;
    memcpy(rsv.data, data, 4);

    if (write(sock, &rsv, sizeof(rsv)) != sizeof(rsv)) {
        perror("send_rsv_relative_Time_command");
    } else {
        printf("상대시간 예약명령 전송 완료 (ID=0x%03X)\n", CMDRSV_ID);
    }
}