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



#define FROM_CAM_ID      0x100

#define CMDPIC_ID       0x031
#define CMDVID_ID       0x033
#define CMDRESET_ID     0x034
#define CMDECHO_ID      0x036
#define CMDRSV_ID       0x039
#define CMDTMSR_ID      0x037
#define CMDTMLR_ID      0x038
#define CMDRSV_ID       0x039
#define CMDLEDPWR_ID    0x050
#define CMD_TMLIGNT_ID  0x051
#define CMD_TMTEMP_ID   0x052



// 경로 설정
#define PIC_DIR   "/home/doteam/Desktop/Camera_team/received_pics"
#define VID_DIR   "/home/doteam/Desktop/Camera_team/received_vids"

#endif
