#include "wireless.h"
#include "protocol.h"

// globals
int					socket_fd;
struct sockaddr_in	server_addr;

int	open_socket(int port)
{
	int	sock;
	int	bind_status;

	sock = socket(AF_INET, SOCK_DGRAM, 0);

	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);

	if (sock != -1)
		bind_status = bind(sock, (struct sockaddr *)&server_addr, sizeof(server_addr));

	if (sock != -1 && bind_status < -1)
	{
		close(sock);
		sock = -1;
	}

	// fcntl(sock, F_SETFL, O_NONBLOCK);

	return (sock);
}

// int	send_keys(int sock, const char *server_ip, circlePosition pad)
// {
// 	int					a;
// 	struct sockaddr_in	server_addr;
//
// 	server_addr.sin_family = AF_INET;
// 	server_addr.sin_port = htons(PORT);
// 	inet_pton(AF_INET, server_ip, &server_addr.sin_addr);
// 	a = sendto(sock, &pad, 4, 0, (struct sockaddr *)&server_addr, sizeof(server_addr));
// 	return (a);
// }

char *discover_server_ip(void)
{
	static char			server_ip[INET_ADDRSTRLEN] = {0};
	int					sock;
	struct sockaddr_in	broadcast_addr = {0}, recv_addr = {0};
	socklen_t			addr_len = sizeof(recv_addr);
	char				message[] = "DISCOVER_SERVER";
	char				recv_buf[64];
	int					broadcast = 1;

	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock < 0)
		return NULL;

	// Enable broadcast
	// int sockopt = setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast));

	// printf("\x1b[1;1H %d - %s", errno, strerror(errno));
	// gfxFlushBuffers();
	// gfxSwapBuffers();

	// memset(&broadcast_addr, 0, sizeof(broadcast_addr));
	broadcast_addr.sin_family = AF_INET;
	broadcast_addr.sin_port = htons(PORT);
	broadcast_addr.sin_addr.s_addr = htonl(INADDR_BROADCAST);

	// Send broadcast message
	sendto(sock, message, sizeof(message), 0,
			(struct sockaddr *) &broadcast_addr,
			sizeof(broadcast_addr)
	);

	// Set timeout for recv
	// struct timeval tv = {2, 0}; // 2 seconds timeout
	// setsockopt(sock, SOL_SOCKET, 0x1006, &tv, sizeof(tv));

	// Set socket to non-blocking
	// int flags = fcntl(sock, F_GETFL, 0);
	// if (flags != -1) {
	// 	fcntl(sock, F_SETFL, flags | O_NONBLOCK);
	// }
	close(sock);

	// Wait for response
	int n = -1, i = 0;

	int sock_recv = socket(AF_INET, SOCK_DGRAM, 0);
	struct sockaddr_in local_addr = {0};
	local_addr.sin_family = AF_INET;
	local_addr.sin_port = htons(6000); // Porta desejada
	local_addr.sin_addr.s_addr = htonl(INADDR_ANY); // Escuta em todas as interfaces

	printf("\x1b[1;1H %d - %s", errno, strerror(errno));
	gfxFlushBuffers();
	gfxSwapBuffers();

	fcntl(sock_recv, F_SETFL, O_NONBLOCK);
	bind(sock_recv, (struct sockaddr *)&local_addr, sizeof(local_addr));

	addr_len = sizeof(local_addr);
	// while (n == -1 && i++ < 5)
	struct pollfd pfd;
	pfd.fd = sock_recv;
	pfd.events = POLLIN;
	int poll_ret = poll(&pfd, 1, 2000);
	if ((pfd.revents & POLLIN) == 0)
		return NULL;
	n = recvfrom(sock_recv, recv_buf, sizeof(recv_buf) - 1, 0, (struct sockaddr *) &local_addr, &addr_len);

	recv_buf[n] = '\0';
	// Optionally check if response is expected
	inet_ntop(AF_INET, &local_addr.sin_addr, server_ip, sizeof(server_ip));
	close(sock_recv);
	return server_ip;
}

void	send_input(int socket_fd, char *server_ip, packet packet_input)
{
	struct sockaddr_in	server_addr;

	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(PORT);
	inet_pton(AF_INET, server_ip, &server_addr.sin_addr);
	sendto(
		socket_fd,
		&packet_input,
		sizeof(packet),
		0,
		(struct sockaddr *)&server_addr,
		sizeof(server_addr)
	);
}
