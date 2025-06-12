#include functions.h
#include common.h

void check_ack() {
    int check = receive_ack(s);

    switch (check) {
        case 1:
            printf("ACK(0x01) 수신됨. 정상입니다.\n");
            break;

        case 2:
            printf("NACK(0x02) 수신됨: 잘못된 명령이 전달되었습니다.\n");
            printf("Recovery Mode로 진입합니다...\n");
            break;

        case 3:
            printf("NACK(0x03) 수신됨: 사진 촬영에 실패하였습니다.\n");
            printf("Recovery Mode로 진입합니다...\n");
            break;

        case 4:
            printf("NACK(0x04) 수신됨: 파일 열기에 실패하였습니다.\n");
            printf("Recovery Mode로 진입합니다...\n");
            break;

        case 135:
            printf("에코 응답 수신됨 (0x87).\n");
            break;

        default:
            printf("알 수 없는 응답 수신됨: 0x%02X\n", check);
            break;
    }
}
