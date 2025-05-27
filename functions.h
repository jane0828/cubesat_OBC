#ifndef FUNCTIONS_H
#define FUNCTIONS_H

void print_menu();
int setup_can_socket(const char *ifname);
int receive_ack(int sock);


#endif
