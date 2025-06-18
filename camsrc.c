#include "common.h"
#include "functions.h"

// recovery2(): CAN 통신 끊겼을 때
void recovery2(int sock) {
    int i = 0;
    int check;
    while (i < 3) {
        send_echo(sock);
        check = receive_ack(sock);
	i++;
        if (check == 2) printf("%d번째 Echo 돌아오지 않음.\n");
        else printf("Echo %d번째 송수신 완료\n", i);
    }



}

// recovery3(): camera not found
// flow: 에코 3번 -> tmsr 받음 -> 카메라 없다하면 리부트 -> tmsr 다시 받아서 확인
void recovery3(int sock) {
    int i = 0;
    int check;
    while (i < 3) {
        send_echo(sock);
        check = receive_ack(sock);
	if (check == 2) break;
        i++;
	printf("Echo %d번째 송수신 완료\n", i);
	if (i == 3) printf("Echo 3번 주고받기 완료. Telemetry 정보를 요청합니다.\n");
    }

    send_tmsr(sock);
    check = receive_tmsr(sock);

    if (check == 1) {
	printf("카메라 연결 정상\n");
	return;
    }

    if (check == 3) {
	printf("카메라 연결 불량 확인됨.\n");
	printf("5초 뒤 camera zero를 재부팅합니다.\n");
	sleep(5);
	send_reboot_command(sock);
	wait_for_echo(sock);
	printf("카메라 정보를 재확인합니다.\n");
	send_tmsr(sock);
	receive_ack(sock);
	check = receive_tmsr(sock);
	if (check == 1) {
	    printf("카메라 연결이 돌아왔습니다. Recovery 3를 종료합니다.\n");
	    return;
	}
	else {
	    printf("카메라는 여전히 미아입니다. 미션 실패\n");
	    return;
	}
    }
}


// CMDACK
int receive_ack(int sock) {
    struct can_frame frame;


    fd_set read_fds;
    struct timeval timeout;
    timeout.tv_sec = 5;      // 30초 동안 기다림
    timeout.tv_usec = 0;

    FD_ZERO(&read_fds);
    FD_SET(sock, &read_fds);

    int ret = select(sock + 1, &read_fds, NULL, NULL, &timeout);

    if (ret == -1) {
          perror("select() 실패");
          return -1;
    } else if (ret == 0) {
          printf("ACK 수신 타임아웃: 응답이 없습니다.\n");
          printf("CAN 통신 라인을 확인하십시오.\n");
          return 2;
    }


    int nbytes = read(sock, &frame, sizeof(struct can_frame));

    if (nbytes < 0) {
        perror("CAN read failed");
        return -1;
    }

    switch (frame.can_id) {
        case 0x100:  //카메라 TC 응답
            if (frame.data[0] == CMDHEL_ID) {
                printf("카메라 설정 성공\n");
                return 1;
            } 
            
            else if (frame.data[0] == CMDPIC_ID) {

                if (frame.data[1] == 0x01) {
                    printf("ACK 수신, 정상입니다!!!!\n");
                    return 1;
                }
                else if (frame.data[1] == 0x02) {
                    printf("Error code 2 수신\n");
                    return 2;
                }
                else if (frame.data[1] == 0x03) {
                    printf("Error code 3 수신\n");
                    return 3;
                }
                else if (frame.data[1] == 0x04) {
                    printf("Error code 4 수신\n");
                    return 4;
                }
                else return 0;
            } 
            else if (frame.data[0] == CMDVID_ID) {
                printf("사진촬영 명령 성공\n");
                return 1;
            } 
            else if (frame.data[0] == CMDRESET_ID) {
                printf("리셋 명령 성공\n");
                return 1;
            } 
            else if (frame.data[0] == CMDECHO_ID) {
                if (frame.data[1] == 0x87) {
                    printf("에코 성공\n");
                    return 1;
		}
		else return 0;
            }
            else if (frame.data[0] == CMDRSVPIC_ID) {
                printf("사진촬영 예약 명령 성공\n");
                return 1;
            }

	    else if (frame.data[0] == CMDTMSR_ID) {
		if (frame.data[1] == 0x01) {
                    printf("ACK 수신, 정상입니다!!!!\n");
                    return 1;
                }
                else if (frame.data[1] == 0x02) {
                    printf("Error code 2 수신\n");
                    return 2;
                }
                else if (frame.data[1] == 0x03) {
                    printf("Error code 3 수신\n");
                    return 3;
                }
                else if (frame.data[1] == 0x04) {
                    printf("Error code 4 수신\n");
                    return 4;
                }
                else return 0;
	    }

            break;
        case 0x200:  //LED TC 응답
            if (frame.data[0] == CMDLEDPWR_ID) {
                printf("LED ON/OFF 명령 성공\n");
                return 1;
            }
            else if (frame.data[0] == CMD_LEDECHO_ID) {
                printf("LED 에코 성공\n");
                return 1;
            } 
            break;
        case 0x137: //TMSR 응답
        {
            int8_t delayTime;
            uint8_t fps;
            uint8_t resolution;

            delayTime = frame.data[0]; 
            fps = frame.data[1];
            resolution = frame.data[2];

            printf("사진촬영 딜레이 시간: %hhusec\n", delayTime);
            printf("FrameRate 설정: %hhufps\n", fps);
            printf("해상도 설정: %hhu\n", resolution);

            break;

        }
        case 0x138: //TMLR 응답
        {
            uint8_t lens_pos;
            uint8_t denosie;
            uint8_t delayTime;
            uint8_t fps;
            uint8_t resolution;
            uint8_t exposeureMode;
            uint8_t exposeureValue;

            lens_pos = frame.data[0]; 
            denosie = frame.data[1];
            delayTime = frame.data[2];
            fps = frame.data[3];
            resolution = frame.data[4];
            exposeureMode = frame.data[5];
            exposeureValue = frame.data[6];


            printf("렌즈 포지션: %hhu\n", lens_pos);
            printf("Noise reduction filter 설정: %hhu\n", denosie);
            printf("사진촬영 딜레이 시간: %hhusec\n", delayTime);
            printf("비디오 FrameRate 설정: %hhufps\n", fps);
            printf("해상도 설정: %hhu\n", resolution);
            printf("노출모드 설정: %hhu\n", exposeureMode);
            printf("노출게인 설정: %hhu\n", exposeureValue);
            break;
        }
        case 0x251: //TMLIGHT 응답
        {
            float fLight;
            uint32_t nLignt; 
            uint8_t reordered[4];

            // 빅엔디안 → 리틀엔디안 (역순으로 넣기)
            reordered[0] = frame.data[3];
            reordered[1] = frame.data[2];
            reordered[2] = frame.data[1];
            reordered[3] = frame.data[0];

            memcpy(&nLignt, reordered, 4);  // 바이트 → float
            //fLight = (float)nLignt;

            printf("광센서 값: %u\n", nLignt);

            break;
        }
        case 0x252: //TMTEMP 응답
        {
            float fTemp;
            uint32_t nTemp;
            uint8_t reordered[4];

            // 빅엔디안 → 리틀엔디안 (역순으로 넣기)
            reordered[0] = frame.data[3];
            reordered[1] = frame.data[2];
            reordered[2] = frame.data[1];
            reordered[3] = frame.data[0];

            memcpy(&nTemp, reordered, 4);  // 바이트 → UINT32
            fTemp = (float)nTemp/100;

            printf("온도센서 값: %f\n", fTemp);

            break;
        }
        default:
            printf("알 수 없는 메시지 ID: 0x%X\n", frame.can_id);
            return 0;
    }
}


// void check_ack(int sock) {
//     int check = receive_ack(sock);
//     printf("check_ack 함수 진입\n");
//     switch (check) {
//         case 1:
//             printf("ACK(0x01) 수신됨. 정상입니다.\n");
//             break;

//         case 2:
//             printf("NACK(0x02) 수신됨: 잘못된 명령이 전달되었습니다.\n");
//             printf("Recovery Mode로 진입합니다...\n");
//             break;

//         case 3:
//             printf("NACK(0x03) 수신됨: 사진 촬영에 실패하였습니다.\n");
//             printf("Recovery Mode로 진입합니다...\n");
//             break;

//         case 4:
//             printf("NACK(0x04) 수신됨: 파일 열기에 실패하였습니다.\n");
//             printf("Recovery Mode로 진입합니다...\n");
//             break;

//         case 135:
//             printf("에코 응답 수신됨 (0x87).\n");
//             break;

//         default:
//             printf("알 수 없는 응답 수신됨: 0x%02X\n", check);
//             break;
//     }
// }





// CMDPIC
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

int receive_image(int sock) {
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
        if(frame.can_dlc != 8) {
	    if (frame.data[1] == 0x03) {
	    	printf("NACK3 수신됨.\n");
	    	printf("Recovery Mode로 진입합니다...\n");
	    	return 3;
	    }

	    if (frame.data[1] == 0x04) {
	    	printf("NACK4 수신됨.\n");
	    	printf("Recovery Mode로 진입합니다...\n");
	    	return 4;
	    }

	}

        // }
        if (buffer == NULL) {
            buffer_size = 1024;
            buffer = malloc(buffer_size);
            if (!buffer) {
                perror("메모리 초기 할당 실패");
                return 0;
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
                return 0;
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
    return 1;
}

//CMDECHO
int send_echo(int sock) {
    struct can_frame echo;
    echo.can_id = CMDECHO_ID;
    echo.can_dlc = 1;
    echo.data[0] = 0x08;

    write(sock, &echo, sizeof(echo));

    return 0;
}

//CMDHEL
void build_hel_command(uint8_t data[8], uint8_t lens_pos, uint8_t denoise) {
    data[0] = lens_pos;
    data[1] = denoise;

    // 나머지 6바이트는 0으로 초기화
    for (int i = 2; i < 8; i++) {
        data[i] = 0x00;
    }
}
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


//CMDRESET
void send_reboot_command(int sock) {
    struct can_frame cmd = { .can_id = CMDRESET_ID, .can_dlc = 1, .data = {0x01} };
    if (write(sock, &cmd, sizeof(cmd)) != sizeof(cmd)) {
        perror("명령 전송 실패");
    } else {
        printf("명령 전송: 0x001\n");
    }
}


void wait_for_echo(int sock) {
    fd_set read_fds;
    struct timeval timeout;
    struct can_frame frame;

    const int total_wait_sec = 300;
    const int poll_interval_sec = 30;
    int waited = 0;

    printf("재부팅 상태 확인 중 (최대 5분 대기)...\n");
    printf("제로가 부팅을 마칠 때까지 40초 대기 중...\n");

    //  최초 40초 대기 (Echo 전송 안 함)
    while (waited < 40) {
        sleep(1);
        waited++;
        if (waited % 5 == 0) {
            printf(".");
            fflush(stdout);
        }
    }
    printf("\n40초 대기 완료. Echo 요청 시작.\n");

    while (waited < total_wait_sec) {
        // 30초 간격으로 Echo 요청 전송
        if ((waited - 30) % poll_interval_sec == 0) {
            struct can_frame echo;
            echo.can_id = CMDECHO_ID;
            echo.can_dlc = 1;
            echo.data[0] = 0x08;

            if (write(sock, &echo, sizeof(echo)) != sizeof(echo)) {
                perror("Echo 요청 전송 실패");
            } else {
                printf("\n[%d초] Echo 요청 전송 완료\n", waited);
            }
        }

        FD_ZERO(&read_fds);
        FD_SET(sock, &read_fds);
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;

        int ret = select(sock + 1, &read_fds, NULL, NULL, &timeout);
        if (ret > 0 && FD_ISSET(sock, &read_fds)) {
            if (read(sock, &frame, sizeof(frame)) > 0) {
                if (frame.can_id == FROM_CAM_ID && frame.data[0] == 0x36 && frame.data[1] == 0x87) {
                    printf("\n재부팅 후 Echo 응답 수신됨! 재부팅 성공\n");
                    return;
                }
            }
        }

        waited++;
        if (waited % 60 == 0) {
            printf("[%d분 경과]\n", waited / 60);
        }
        if (waited % 5 == 0) {
            printf(".");
            fflush(stdout);
        }
    }

    printf("\nEcho 응답 없음. 재부팅 실패 또는 통신 문제일 수 있음.\n");
}




//CMDRSV
void send_rsvpic(int sock, const uint8_t data[8]) {
    struct can_frame rsv;
    rsv.can_id = CMDRSVPIC_ID;
    rsv.can_dlc = 4;
    memcpy(rsv.data, data, 4);

    if (write(sock, &rsv, sizeof(rsv)) != sizeof(rsv)) {
        perror("send_rsv_utc_command");
    } else {
        printf("사진촬영영 예약명령 전송 완료 (ID=0x%03X)\n", CMDRSVPIC_ID);
    }
}

//CMDVID
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

//TMLR
int send_tmlr(int sock) {
    struct can_frame TMLR;
    TMLR.can_id = CMDTMLR_ID;
    TMLR.can_dlc = 1;
    TMLR.data[0] = 0x00;


    if (write(sock, &TMLR, sizeof(TMLR)) != sizeof(TMLR)) {
        perror("send_tmlr_command");
    } else {
        printf("TMLR 전송 완료 (ID=0x%03X)\n", CMDTMLR_ID);
    }
    

    return 0;
}

//TMSR
int send_tmsr(int sock) {
    struct can_frame TMSR;
    TMSR.can_id = CMDTMSR_ID;
    TMSR.can_dlc = 1;
    TMSR.data[0] = 0x00;


    if (write(sock, &TMSR, sizeof(TMSR)) != sizeof(TMSR)) {
        perror("send_tmsr_command");
    } else {
        printf("TMSR 전송 완료 (ID=0x%03X)\n", CMDTMSR_ID);
    }


    return 0;
}


int receive_tmsr(int sock) {
    struct can_frame frame;
    char camera_info[256] = {0};
    int offset = 0;

    printf("카메라 등록 정보 수신 중...\n");

    while (1) {
        if (read(sock, &frame, sizeof(frame)) > 0) {
            if (frame.can_id == FROM_CAM_ID) {
                // null 종료 조건
                if (frame.data[0] == 0x00) {
                    break;
		}
		if (frame.data[1] == 0x03) {
//		    printf("ERROR CODE 3\n");
//		    printf("카메라 발견되지 않음.\n");
//		    printf("Recovery Mode로 진입합니다...\n");
		    return 3;
                }

                for (int i = 0; i < frame.can_dlc && offset < sizeof(camera_info) - 1; ++i) {
                    camera_info[offset++] = frame.data[i];
                }
            }
        }
    }

    camera_info[offset] = '\0';
    printf("\n[수신 완료] 등록된 카메라 정보:\n%s\n", camera_info);
    return 1;
}

