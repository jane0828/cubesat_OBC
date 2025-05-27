#include "common.h"
#include "functions.h"

void build_camera_command(uint8_t data[8], uint8_t delay_sec, uint32_t shutter_us,
                          uint8_t resolution, uint8_t exposure_mode, int8_t exposure_value) {
    data[0] = delay_sec;
    data[1] = (shutter_us >> 24) & 0xFF;
    data[2] = (shutter_us >> 16) & 0xFF;
    data[3] = (shutter_us >> 8) & 0xFF;
    data[4] = shutter_us & 0xFF;
    data[5] = resolution;
    data[6] = exposure_mode;
    data[7] = (uint8_t)exposure_value;
}

void send_camera_command(int sock, const uint8_t data[8]) {
    struct can_frame cam;
    cam.can_id = CMDPIC_ID;
    cam.can_dlc = 8;
    memcpy(cam.data, data, 8);

    if (write(sock, &cam, sizeof(cam)) != sizeof(cam)) {
        perror("send_command");
    } else {
        printf("카메라 명령 전송 완료 (ID=0x%03X)\n", CMDPIC_ID);
    }
}

void receive_image(int sock) {
    struct can_frame frame;
    unsigned char *buffer = NULL;
    size_t buffer_size = 0;
    size_t offset = 0;

    printf("이미지 수신 대기 중 (CAN ID 0x%03X)...\n", FROM_CAM_ID);

    struct timeval start_time, end_time;
    int started = 0;

    // 파일명 생성
    char filename[256];
    char fullpath[512];
    time_t now = time(NULL);
    struct tm *t = localtime(&now);

    snprintf(filename, sizeof(filename),
            "received_photo_%04d%02d%02d_%02d%02d%02d.jpg",
            t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
            t->tm_hour, t->tm_min, t->tm_sec);

    snprintf(fullpath, sizeof(fullpath), "%s/%s", PIC_DIR, filename);

    while (1) {
        if (read(sock, &frame, sizeof(frame)) <= 0) continue;
        if (frame.can_id != FROM_CAM_ID) continue;
        // if(frame.can_dlc != 8) {
        //     printf("Read Byte : %d\n", res);
        //     printf("Data Size : %d\n", frame.can_dlc);

        // }
        if (buffer == NULL) {
            buffer_size = 1024;
            buffer = malloc(buffer_size);
            if (!buffer) {
                perror("메모리 초기 할당 실패");
                return;
            }
            memset(buffer, 0, buffer_size); 
        }


        // 수신 시작 시간 기록
        if (!started) {
            gettimeofday(&start_time, NULL);
            started = 1;
        }

        // 종료 프레임 수신
        if (frame.can_dlc == 1 && frame.data[0] == 0xFF) {
            printf("전송 종료 프레임 수신\n");
            break;
        }

        // 메모리 재할당
        size_t needed = offset + frame.can_dlc;
        if (needed > buffer_size) {
            size_t old_size = buffer_size;
            buffer_size = needed + 1024;

            unsigned char *new_buf = realloc(buffer, buffer_size);
            if (!new_buf) {
                perror("메모리 재할당 실패");
                free(buffer);
                return;
            }

            buffer = new_buf;

            // 새로 확장된 부분 0으로 초기화
            memset(buffer + old_size, 0, buffer_size - old_size);
        }

        memcpy(buffer + offset, frame.data, frame.can_dlc);
        offset += frame.can_dlc;
    }

    // 수신 완료 시간 측정
    gettimeofday(&end_time, NULL);
    long elapsed_sec = end_time.tv_sec - start_time.tv_sec;
    long elapsed_usec = end_time.tv_usec - start_time.tv_usec;
    double elapsed_ms = elapsed_sec * 1000.0 + elapsed_usec / 1000.0;

    // 파일 저장
    FILE *fp = fopen(fullpath, "wb");
    if (fp) {
        fwrite(buffer, 1, offset, fp);
        fclose(fp);
        printf("이미지 저장 완료: %s (%zu bytes)\n", fullpath, offset);
    } else {
        perror("파일 저장 실패");
    }

    printf("총 수신 시간: %.2f ms (%.2f 초)\n", elapsed_ms, elapsed_ms / 1000.0);
    free(buffer);
}