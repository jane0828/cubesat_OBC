#include "common.h"
#include "functions.h"
#include "camsrc.h"
#include "ledsrc.h"
#include <time.h>

int main() {
    int s = setup_can_socket("can0");
    int choice;
    int check;
    int val;

    while (1) {
        print_menu();
        scanf("%d", &choice);

        switch (choice) {
            case 0:
                printf("종료합니다.\n");
                return 0;
//            case 1: //CMDHEL
//                uint8_t lens_pos, denoise;

//                printf("카메라 렌즈 위치를 설정하시오.\n");
//                printf("위치 [mm]: ");
//                scanf("%hhu", &lens_pos);
//
//                printf("카메라 노이즈 설정하시오. Noise reduction filter 설정, 0x00 : auto (기본값), 0x01 : cdn_off, 0x02 : cdn_fast, 0x03 : cdn_hq ");
//                scanf("%u", &denoise);

//                uint8_t cmd[6];
//                build_hel_command(cmd, lens_pos, denoise);
//                send_hel_command(s, cmd);
                
//                break;

            case 2:  //CMDPIC & CMDSEND
                uint8_t delay, resolution, mode;
                uint32_t shutter;
                int8_t ev;

                printf("사진 촬영 파라미터를 입력하세요.\n");
                printf("딜레이 시간 [s]: ");
                scanf("%hhu", &delay);

                printf("셔터 속도 [ms] (recommended: 5000): ");
                scanf("%u", &shutter);

                printf("해상도 (0:1080p, 1:XGA, 2:WXGA, 3:720p, 4:초저화질): ");
                scanf("%hhu", &resolution);

                printf("노출 모드 (0:normal, 1:sport, 2:long): ");
                scanf("%hhu", &mode);

                printf("노출 보정값 (-8 ~ +8): ");
                scanf("%hhd", &ev);

                uint8_t cmd_cam[8];
                build_camera_command(cmd_cam, delay, shutter, resolution, mode, ev);
                send_camera_command(s, cmd_cam);
                check = receive_ack(s);
                if (check != 1) break;
		val = receive_image(s);
		if (val == 1) {
		    receive_ack(s);
		    break;
		}

		else if (val == 2) {
		    printf("Error Code 2 Received.\n");
		    printf("Recovery Mode 2 Activated.\n");
		    break;
		}
                else if (val == 3) {
                    printf("Error Code 3 Received.\n");
                    printf("Recovery Mode 3 Activated.\n");
                    break;
		}
                else if (val == 4) {
                    printf("Error Code 4 Received.\n");
                    printf("Recovery Mode 4 Activated.\n");
		    break;
                }
		else {
		    printf("...Sorry...\n");
		    break;
		}

            case 3: //CMDVID & CMDSEND
                uint32_t delay_ms;
                uint8_t fps;

                printf("영상 촬영 파라미터를 입력하세요.\n");
                printf("촬영 시간 [ms] (recommended: 5000): ");
                scanf("%u", &delay_ms);

                printf("Frames Per Second(FPS) (recommended: 30): ");
                scanf("%hhu", &fps);

                uint8_t vcmd[8];
                build_video_command(vcmd, delay_ms, fps);
                send_video_command(s, vcmd);
                receive_video(s, fps);
                break;


            case 4: //CMDRESET
                send_reboot_command(s);
		check = receive_ack(s);
		if (check != 1) break;
                wait_for_echo(s);
                break;

            case 5: //CMDECHO
                send_echo(s);
                receive_ack(s);
                break;

            case 6: //TMSR
                send_tmsr(s);
		check = receive_ack(s);
		if (check != 1) break;
		val = receive_tmsr(s);
                if (val == 1) {
                    receive_ack(s);
                    break;
                }

                else if (val == 2) {
                    printf("Error Code 2 Received.\n");
                    printf("Recovery Mode 2 Activated.\n");
                    break;
                }
                else if (val == 3) {
                    printf("Error Code 3 Received.\n");
                    printf("Recovery Mode 3 Activated.\n");
                    break;
                }
                else if (val == 4) {
                    printf("Error Code 4 Received.\n");
                    printf("Recovery Mode 4 Activated.\n");
                    break;
                }
                else {
                    printf("...Sorry...\n");
                    break;
                }

            case 7: //TMLR
                send_tmlr(s);
                receive_ack(s);
                break;
            case 8: //CMDRSVPIC
            {
                uint32_t unixtime;
                printf("UNIX TIME 입력.\n");
                scanf("%u", &unixtime);

                // 현재 시간 (Unix timestamp: 초 단위)
                time_t unix_time = time(NULL);
                if (unix_time == ((time_t)-1)) {
                    perror("time error");
                    return 1;
                }

                // Unix timestamp 출력
                printf("Current Unix time: %ld\n", unix_time);

                // struct tm 포인터로 변환
                struct tm *local_time = localtime(&unix_time);
                if (local_time == NULL) {
                    perror("localtime error");
                    return 1;
                }

                // 문자열로 포맷팅
                char buffer[100];
                if (strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", local_time)) {
                    printf("Local time: %s\n", buffer);
                } else {
                    printf("strftime error\n");
                }

                uint8_t unixtimebuffer[8] = {0};
                // big-endian으로 저장 (상위 바이트부터)
                unixtimebuffer[0] = (unixtime >> 24) & 0xFF;  // MSB
                unixtimebuffer[1] = (unixtime >> 16) & 0xFF;
                unixtimebuffer[2] = (unixtime >> 8)  & 0xFF;
                unixtimebuffer[3] = (unixtime)       & 0xFF;  // LSB

                send_rsvpic(s, unixtimebuffer);
                receive_ack(s);

            }
                break;
            case 9: //CMDLEDPWR
                uint8_t onoff;
                uint8_t cmd_led[8];
                printf("LED 전체 ON/OFF 제어");
                printf("ON: 1, OFF: 0 \n");
                scanf("%hhu", &onoff);
                cmd_led[0] = onoff;
                send_led_pwr(s, cmd_led);
                receive_ack(s);
                break;
            case 10: //TMLIGHT
                send_tmlight(s);
                receive_ack(s);
                break;
            case 11: //TMTEMP
                send_tmtemp(s);
                receive_ack(s);
                break;
            case 12: //LEDECHO
                send_led_echo(s);
                receive_ack(s);
                break;

            default:
                printf("유효하지 않은 선택입니다.\n");
        }
    }
    return 0;
}
