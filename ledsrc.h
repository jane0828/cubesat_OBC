#ifndef LEDSRC_H
#define LEDSRC_H

int send_tmlight(int sock);
int send_tmtemp(int sock);
int send_led_echo(int sock);
int send_led_pwr(int sock, const uint8_t data[8]);

#endif