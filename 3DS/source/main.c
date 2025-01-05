#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <errno.h>

#include <3ds.h>
#include "drawing.h"
#include "wireless.h"
#include "protocol.h"

int main(void)
{
	acInit();
	gfxInitDefault();

	gfxSetDoubleBuffering(GFX_TOP, false);
	gfxSetDoubleBuffering(GFX_BOTTOM, false);

	clearScreen();
	drawString(10, 10, "Initialising FS...");
	gfxFlushBuffers();
	gfxSwapBuffers();

	fsInit();

	clearScreen();
	drawString(10, 10, "Initialising SOC...");
	gfxFlushBuffers();
	gfxSwapBuffers();

	// Aqui inicializa o serviço do socket
	socInit((u32 *)memalign(0x1000, 0x100000), 0x100000);

	while (aptMainLoop()) /* Wait for WiFi; break when WiFiStatus is truthy */
	{
		u32 wifiStatus = 0;
		ACU_GetWifiStatus(&wifiStatus);
		if (wifiStatus)
			break;

		hidScanInput();
		clearScreen();
		drawString(10, 10, "Waiting for WiFi connection...");
		drawString(10, 20, "Ensure you are in range of an access point,");
		drawString(10, 30, "and that wireless communications are enabled.");
		drawString(10, 50, "You can alternatively press Start and Select to exit.");

		u32 keys = hidKeysHeld();
		if ((keys & KEY_START) && (keys & KEY_SELECT))
			goto exit;

		gfxFlushBuffers();
		gspWaitForVBlank();
		gfxSwapBuffers();
	}

	clearScreen();
	drawString(10, 10, "Reading settings...");
	gfxFlushBuffers();
	gfxSwapBuffers();

	// Aqui lê o arquivo 3DSController.ini e seta as configurações
	// O IP de quem vai receber os inputs está aqui
	// if (!readSettings()) {
	// 	hang("Could not read 3DSController.ini!");
	// }

	clearScreen();
	// drawString(10, 10, "Connecting to %s on port %d...", settings.IPString, settings.port);
	gfxFlushBuffers();
	gfxSwapBuffers();

	// Aqui abre o socket na porta 6000
	socket_fd = open_socket(6000);

	// Aqui manda para o cliente que quer se conectar
	// sendConnectionRequest();

	clearScreen();
	gfxFlushBuffers();
	gfxSwapBuffers();

	disableBacklight();

	// Sleep for 2 seconds
	// svcSleepThread(2000000000LL);

	while (aptMainLoop())
	{
		u32				keys;
		// touchPosition	touch;
		circlePosition	circlePad;
		// circlePosition	cStick;
		payload			packet_payload_input;
		packet			packet_input;

		hidScanInput();
		// irrstScanInput();

		keys = hidKeysHeld();
		hidCircleRead(&circlePad);
		// hidCstickRead(&cStick);
		// touchRead(&touch);

		// clearScreen();
		// drawString(10, 10, "socket %d", socket_fd);
		// drawString(10, 20, "circlePad: %i %i", circlePad.dx, circlePad.dy);
		// drawString(10, 50, "Press Start and Select to exit.");

		// Aqui manda os inputs para o cliente
		// sendKeys(keys, circlePad, touch, cStick);
		// send_keys(socket_fd, IP, circlePad);
		packet_payload_input = transform_input_for_switch(keys, circlePad);

		packet_input.magic_number = MAGIC_NUMBER;
		packet_input.packet_size = sizeof(packet);
		packet_input.packet_type = HAS_BUTTONS | HAS_LEFT_ANALOG;
		packet_input.payload = packet_payload_input;

		send_input(socket_fd, IP, packet_input);

		//receiveBuffer(sizeof(struct packet));

		if ((keys & KEY_START) && (keys & KEY_SELECT))
			break ;

		gfxFlushBuffers();
		gspWaitForVBlank();
		gfxSwapBuffers();
	}

	exit:

	enableBacklight();

	packet	packet_input;
	packet_input.magic_number = 0;
	packet_input.packet_size = sizeof(packet);
	packet_input.packet_type = HAS_BUTTONS | HAS_LEFT_ANALOG;
	memset(&packet_input.payload, 0, sizeof(payload));

	send_input(socket_fd, IP, packet_input);

	// Daqui para baixo para tudo e fecha
	SOCU_ShutdownSockets();

	// Isso fecha alguma coisa relacionada a leitura de arquivo
	// svcCloseHandle(fileHandle);
	fsExit();

	gfxExit();
	acExit();

	return 0;
}
