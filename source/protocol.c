#include "protocol.h"

Result	init_all(void)
{
	Result	rc_buffer = 0;
	Result	rc_device = 0;
	Result	rc_bind = 0;

	controller_state.battery_level = 4;
	rc_buffer = hiddbgAttachHdlsWorkBuffer(&session_id, workmem, workmem_size);
	if (R_SUCCEEDED(rc_buffer))
		rc_device = hiddbgAttachHdlsVirtualDevice(&controller_handle, &controller_device);
		// Aqui vai mudar
		// Ao invés de criar um controle virtual ao iniciar o symodule, ele vai criar apenas quando receber
		// os dados do 3ds e deletar quando o 3ds parar de mandar os dados
	else
		return (1);

	if (R_FAILED(rc_device))
	{
		hiddbgReleaseHdlsWorkBuffer(session_id);
		return (1);
	}

	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = INADDR_ANY;
	servaddr.sin_port = htons(PORT);

	if ((socket_fd = socket(AF_INET, SOCK_DGRAM, 0)) != -1)
		rc_bind = bind(socket_fd, (struct sockaddr *)&servaddr, sizeof(servaddr));

	if (socket_fd == -1 || rc_bind != 0)
	{
		if (socket_fd != -1)
			close(socket_fd);

		hiddbgDetachHdlsVirtualDevice(controller_handle);
		hiddbgReleaseHdlsWorkBuffer(session_id);

		return (1);
	}

	return (0);
}

void	finalize(void)
{
	close(socket_fd);
	hiddbgDetachHdlsVirtualDevice(controller_handle);
	hiddbgReleaseHdlsWorkBuffer(session_id);
}
