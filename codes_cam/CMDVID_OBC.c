#include "common.h"
#include "functions.h"

void build_video_command(uint8_t data[8], uint32_t delay_ms, uint8_t fps) {
    data[0] = (delay_ms >> 24) & 0xFF;
    data[1] = (delay_ms >> 16) & 0xFF;
    data[2] = (delay_ms >> 8) & 0xFF;
    data[3] = delay_ms & 0xFF;
    data[4] = fps;
    for (int i = 5; i < 8; i++) data[i] = 0;  // 나머지 채움
}

void send_video_command(int sock, const uint8_t data[8]) {
    struct can_frame cmd;
    cmd.can_id = CMDVID_ID;
    cmd.can_dlc = 8;
    memcpy(cmd.data, data, 8);

    if (write(sock, &cmd, sizeof(cmd)) != sizeof(cmd)) {
        perror("send_video_command");
    } else {
        printf("영상 촬영 명령 전송 완료 (ID=0x%03X)\n", CMDVID_ID);
    }
}

void receive_video(int sock, uint8_t fps) {
    struct can_frame frame;
    unsigned char *buffer = NULL;
    size_t buffer_size = 0;
    size_t offset = 0;

    printf("영상 수신 대기 중 (CAN ID 0x%03X)...\n", FROM_CAM_ID);

    struct timeval start_time, end_time;
    int started = 0;

    char filename[256];
    char fullpath[512];
    time_t now = time(NULL);
    struct tm *t = localtime(&now);

    snprintf(filename, sizeof(filename),
            "received_video_%04d%02d%02d_%02d%02d%02d.h264",
            t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
            t->tm_hour, t->tm_min, t->tm_sec);
    snprintf(fullpath, sizeof(fullpath), "%s/%s", VID_DIR, filename);

    while (1) {
        if (read(sock, &frame, sizeof(frame)) <= 0) continue;
        if ((frame.can_id & 0x7FF) != (FROM_CAM_ID)) continue;

        if (frame.can_dlc == 1 && frame.data[0] == 0xFF) {
            printf("전송 종료 프레임 수신\n");
            break;
        }

        if (!started) {
            gettimeofday(&start_time, NULL);
            buffer_size = 1024;
            buffer = malloc(buffer_size);
            if (!buffer) {
                perror("메모리 할당 실패");
                return;
            }
            memset(buffer, 0, buffer_size);
            started = 1;
        }

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
            memset(buffer + old_size, 0, buffer_size - old_size);
        }

        memcpy(buffer + offset, frame.data, frame.can_dlc);
        offset += frame.can_dlc;
    }

    gettimeofday(&end_time, NULL);
    long elapsed_sec = end_time.tv_sec - start_time.tv_sec;
    long elapsed_usec = end_time.tv_usec - start_time.tv_usec;
    double elapsed_ms = elapsed_sec * 1000.0 + elapsed_usec / 1000.0;

    FILE *fp = fopen(fullpath, "wb");
    if (fp) {
        fwrite(buffer, 1, offset, fp);
        fclose(fp);
        printf("영상 저장 완료: %s (%zu bytes)\n", fullpath, offset);
    } else {
        perror("파일 저장 실패");
    }

    printf("총 수신 시간: %.2f ms (%.2f 초)\n", elapsed_ms, elapsed_ms / 1000.0);
    free(buffer);

    char mp4_path[512];
    snprintf(mp4_path, sizeof(mp4_path), "%s.mp4", fullpath);  // 확장자만 추가

    char cmd[1024];
    snprintf(cmd, sizeof(cmd),
            "ffmpeg -y -framerate %u -i %s -c copy %s > /dev/null 2>&1",
            fps, fullpath, mp4_path);

    printf("MP4 변환 중...\n");
    int ret = system(cmd);
    if (ret == 0) {
        printf("MP4 저장 완료: %s\n", mp4_path);
        // .h264 삭제 가능
        remove(fullpath);
    } else {
        printf("MP4 변환 실패 (ffmpeg 에러)\n");
    }


}