#ifndef CAMSRC_H
#define CAMSRC_H

// CMDACK
int receive_ack(int sock);

// CMDPIC
void build_camera_command(uint8_t data[8], uint8_t delay_sec, uint32_t shutter_us,
                          uint8_t resolution, uint8_t exposure_mode, int8_t exposure_value);
void send_camera_command(int sock, const uint8_t data[8]);
int receive_image(int sock);

//CMDECHO
int send_echo(int sock);

//CMDHEL
void build_hel_command(uint8_t data[8], uint8_t lens_pos, uint8_t denoise);
void send_hel_command(int sock, const uint8_t data[8]);

//CMDRESET
void send_reboot_command(int sock);
void wait_for_echo(int sock);

//CMDRSV
void send_rsvpic(int sock, const uint8_t data[8]);

//CMDVID
void build_video_command(uint8_t data[8], uint32_t delay_ms, uint8_t fps);
void send_video_command(int sock, const uint8_t data[8]);
void receive_video(int sock, uint8_t fps);


//TMLR
int send_tmlr(int sock);

//TMSR
int send_tmsr(int sock);
int receive_tmsr(int sock);

#endif
