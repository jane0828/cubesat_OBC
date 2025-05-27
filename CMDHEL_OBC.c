#include "common.h"
#include "functions.h"

/*
  @brief 헬(HEL) 명령을 위한 데이터 구성
  
  @param data     8바이트 CAN 데이터 프레임
  @param lens_pos 카메라 렌즈 위치 설정값 (0 ~ 32)
  @param denoise  노이즈 필터 설정값
                  0x00: auto (기본값)
                  0x01: cdn_off
                  0x02: cdn_fast
                 0x03: cdn_hq
 */
void build_hel_command(uint8_t data[8], uint8_t lens_pos, uint8_t denoise) {
    data[0] = lens_pos;
    data[1] = denoise;

    // 나머지 6바이트는 0으로 초기화
    for (int i = 2; i < 8; i++) {
        data[i] = 0x00;
    }
}

/*
 @brief HEL 명령 전송 함수
  
 @param sock     CAN 소켓
 @param data     위에서 구성한 8바이트 데이터
 */
void send_hel_command(int sock, const uint8_t data[8]) {
    struct can_frame hel;
    hel.can_id = CMDHEL_ID;   // common.h에 정의된 ID 사용
    hel.can_dlc = 8;
    memcpy(hel.data, data, 8);

    if (write(sock, &hel, sizeof(hel)) != sizeof(hel)) {
        perror("send_hel_command");
    } else {
        printf("HEL 명령 전송 완료 (ID=0x%03X)\n", CMDHEL_ID);
    }
}