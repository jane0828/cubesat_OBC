#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <time.h>

#include <linux/can.h>
#include <linux/can/raw.h>

#include <net/if.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/time.h>



//#define CMD_ID     0x200
//#define DATA_ID    0x300
//#define END_ID     0x3FF
//#define NAME_ID    0x3FE
//#define ECHO_ID    0x36
//#define STATUS_ID  0x2FF
//#define CAMERA_CMD_ID 0x106
//#define VIDEO_CMD_ID 0x120

#define END_FRAME_VAL  0xFF
#define MAX_CAN_DLC    8


#define FROM_CAM_ID      0x100

#define CMDHEL_ID       0x030
#define CMDPIC_ID       0x031
#define CMDVID_ID       0x033
#define CMDRESET_ID     0x034
#define CMDECHO_ID      0x036
#define CMDTMSR_ID      0x037
#define CMDTMLR_ID      0x038
#define CMDRSVPIC_ID    0x039
#define CMDLEDPWR_ID    0x050
#define CMD_TMLIGNT_ID  0x051
#define CMD_TMTEMP_ID   0x052
#define CMD_LEDECHO_ID  0x055

//#define FLAG_PIC        0x001
//#define FLAG_VID        0x002
//#define FLAG_ACK        0x004
//#define FLAG_SEND       0x008



// 경로 설정
#define PIC_DIR   "/home/doteam/Desktop/Camera_team/received_pics"
#define VID_DIR   "/home/doteam/Desktop/Camera_team/received_vids"

#endif
