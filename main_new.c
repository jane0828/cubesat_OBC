#include "common.h"
#include "functions.h"
#include "camsrc.h"
#include "ledsrc.h"
#include <time.h>
#include <stdbool.h>
#include <unistd.h>   // sleep()

// FDIR 헬퍼
static void can_manual_up(int s) {
    // TODO: CAN 인터페이스 재설정 또는 재오픈 로직 구현
}

static void soft_reboot() {
    // TODO: OBC 소프트 리부트 호출 (예: system("reboot"); )
}

int main() {
    int s = setup_can_socket("can0");
    int choice;

    while (1) {
        print_menu();
        scanf("%d", &choice);

        switch (choice) {
            case 0:
                printf("종료합니다.\n");
                return 0;

            case 1: { // CMDHEL
                uint8_t lens_pos, denoise;

                printf("카메라 렌즈 위치를 설정하시오.\n");
                printf("위치 [mm]: ");
                scanf("%hhu", &lens_pos);

                printf(
                    "카메라 노이즈 설정하시오. Noise reduction filter 설정,"
                    " 0x00:auto, 0x01:cdn_off, 0x02:cdn_fast, 0x03:cdn_hq ");
                scanf("%hhu", &denoise);

                uint8_t cmd[8] = {0};
                build_hel_command(cmd, lens_pos, denoise);
                send_hel_command(s, cmd);
                break;
            }

            case 2: { // CMDPIC & FDIR 적용
                uint8_t  delay, resolution, mode;
                uint32_t shutter;
                int8_t   ev;

                printf("사진 촬영 파라미터를 입력하세요.\n");
                printf("딜레이 시간 [s]: "); scanf("%hhu", &delay);
                printf("셔터 속도 [ms] (recommended: 5000): "); scanf("%u", &shutter);
                printf("해상도 (0:1080p,1:XGA,2:WXGA,3:720p,4:초저화질): "); scanf("%hhu", &resolution);
                printf("노출 모드 (0:normal,1:sport,2:long): "); scanf("%hhu", &mode);
                printf("노출 보정값 (-8~+8): "); scanf("%hhd", &ev);

                uint8_t cmd_cam[8];
                build_camera_command(cmd_cam, delay, shutter, resolution, mode, ev);

                // 1) CAN 명령 전송
                send_camera_command(s, cmd_cam);

                // 2) FDIR 알고리즘
                bool comm_ok  = false;
                int  nack_cnt = 0, echo_cnt = 0, can_cnt = 0;
                const int MAX_NACK = 3, MAX_ECHO = 3, MAX_CAN = 3;
                int  code;

                // 최초 ACK/NACK 검사
                code = receive_ack(s);
                if (code == 1 || code == 135) {
                    comm_ok = true;
                } else if (code > 1) {
                    nack_cnt++;
                }

                // 복구 루프
                while (!comm_ok) {
                    // (a) ECHO 5초 간격 3회
                    for (echo_cnt = 0; echo_cnt < MAX_ECHO && !comm_ok; ++echo_cnt) {
                        send_echo(s);
                        sleep(5);
                        code = receive_ack(s);
                        if (code == 1 || code == 135) comm_ok = true;
                        else if (code > 1 && ++nack_cnt > MAX_NACK) break;
                    }
                    if (comm_ok) break;

                    // (b) CAN 수동 UP
                    can_manual_up(s);

                    // (c) ECHO 재시도
                    for (echo_cnt = 0; echo_cnt < MAX_ECHO && !comm_ok; ++echo_cnt) {
                        send_echo(s);
                        sleep(5);
                        code = receive_ack(s);
                        if (code == 1 || code == 135) comm_ok = true;
                    }
                    if (comm_ok) break;

                    // (d) CAN 재시도 한계
                    if (++can_cnt > MAX_CAN) {
                        // UART 폴백 (주석)
                        // TODO: uart_send_camera(cmd_cam) + ACK 처리
                        break;
                    }
                }

                // 3) 결과 처리
                if (comm_ok) {
                    receive_image(s);
                    receive_ack(s);
                    check_ack(s);
                } else {
                    soft_reboot();
                }
            } break;

            case 3: // CMDVID & CMDSEND
                {
                    uint32_t delay_ms;
                    uint8_t fps;
                    printf("영상 촬영 파라미터를 입력하세요.\n");
                    printf("촬영 시간 [ms] (recommended: 5000): "); scanf("%u", &delay_ms);
                    printf("Frames Per Second(FPS) (recommended: 30): "); scanf("%hhu", &fps);
                    uint8_t vcmd[8];
                    build_video_command(vcmd, delay_ms, fps);
                    send_video_command(s, vcmd);
                    receive_video(s, fps);
                }
                break;

            case 4: // CMDRESET
                send_reboot_command(s);
                receive_ack(s);
                break;

            case 5: // CMDECHO
                send_echo(s);
                receive_ack(s);
                break;

            case 6: // TMSR
                send_tmsr(s);
                receive_ack(s);
                break;

            case 7: // TMLR
                send_tmlr(s);
                receive_ack(s);
                break;

            case 8: // CMDRSVPIC
                {
                    uint32_t unixtime;
                    printf("UNIX TIME 입력.\n");
                    scanf("%u", &unixtime);
                    send_rsvpic(s, (uint8_t[]){
                        (uint8_t)(unixtime>>24), (uint8_t)(unixtime>>16),
                        (uint8_t)(unixtime>>8),  (uint8_t)(unixtime)
                    });
                    receive_ack(s);
                }
                break;

            case 9: // CMDLEDPWR
                {
                    uint8_t onoff;
                    printf("LED 전체 ON/OFF 제어 (1:ON, 0:OFF)\n");
                    scanf("%hhu", &onoff);
                    send_led_pwr(s, (uint8_t[]){onoff});
                    receive_ack(s);
                }
                break;

            case 10: // TMLIGHT
                send_tmlight(s);
                receive_ack(s);
                break;

            case 11: // TMTEMP
                send_tmtemp(s);
                receive_ack(s);
                break;

            case 12: // LEDECHO
                send_led_echo(s);
                receive_ack(s);
                break;

            default:
                printf("유효하지 않은 선택입니다.\n");
        }
    }

    return 0;
}

