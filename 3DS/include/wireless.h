#ifndef WIRELESS_H
# define WIRELESS_H

// defines
// # define IP		"192.168.15.105"
# define PORT	6000

//----------------------------------------------------------------------------//
//                      includes                     //
//----------------------------------------------------------------------------//

#include <errno.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <poll.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <3ds.h>
#include "protocol.h"

// variaveis
extern int					socket_fd;
extern struct sockaddr_in	server_addr;

// funções
int		open_socket(int port);
char	*discover_server_ip(void);
// int		send_keys(int sock, const char *server_ip, circlePosition pad);
void	send_input(int socket_fd, char *server_ip, packet packet_input);

#endif
