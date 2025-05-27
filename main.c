#include "common.h"
#include "functions.h"

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
            case 1:
                uint8_t lens_pos, denoise;

                printf("카메라 렌즈 위치를 설정하시오.\n");
                printf("위치 [mm]: ");
                scanf("%hhu", &lens_pos);

                printf("카메라 노이즈 설정하시오. Noise reduction filter 설정, 0x00 : auto (기본값), 0x01 : cdn_off, 0x02 : cdn_fast, 0x03 : cdn_hq ");
                scanf("%u", &denoise);

                uint8_t cmd[6];
                build_hel_command(cmd, lens_pos, denoise);
                send_hel_command(s, cmd);
                
                break;

            case 2: 
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
                receive_image(s);
                break;
            

            case 3:
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


            // case 4:
            //     send_reboot_command(s);
            //     receive_ack(s);
            //     break;

            case 5:
                send_echo(s);
                receive_ack(s);
                break;

            case 6:
                
                break;

            default:
                printf("유효하지 않은 선택입니다.\n");
        }
    }
    return 0;
}
