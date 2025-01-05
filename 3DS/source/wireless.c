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
