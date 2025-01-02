#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "protocol.h"

// Size of the inner heap (adjust as necessary).
#define INNER_HEAP_SIZE 0x80000

//---------------------------------------------------//
//                      globals                      //
//---------------------------------------------------//

u32						__nx_applet_type = AppletType_None;
u32						__nx_fs_num_sessions = 1;

// Initialization flag
bool					init_flag = 0;

// Work memory
u8						*workmem = NULL;
size_t 					workmem_size = 0x1000;

// Controller session and handle
HiddbgHdlsSessionId		session_id = {0};
HiddbgHdlsHandle		controller_handle = {0};

// Controller type and state
HidDeviceType			controller_initialized_type = HidDeviceType_FullKey3;
HiddbgHdlsState			controller_state = {0};

// Controller device information
HiddbgHdlsDeviceInfo	controller_device =
{
	.deviceType			= HidDeviceType_FullKey3, 			// Pro Controller
	.npadInterfaceType	= HidNpadInterfaceType_Bluetooth,
	.singleColorBody	= RGBA8_MAXALPHA(255, 255, 255),
	.singleColorButtons	= RGBA8_MAXALPHA(0, 0, 0),
	.colorLeftGrip		= RGBA8_MAXALPHA(230, 255, 0),
	.colorRightGrip		= RGBA8_MAXALPHA(0, 40, 20),
	.pad				= {0}
};

// Socket
int								socket_fd = -1;
struct sockaddr_in				servaddr;
struct sockaddr_in				cliaddr;
static const SocketInitConfig	sockInitConf =
{
	.udp_tx_buf_size = 0x2600,
	.udp_rx_buf_size = 0xA700,
	.sb_efficiency = 2,
	.num_bsd_sessions = 3,
	.bsd_service_type = BsdServiceType_User
};

//---------------------------------------------------//
//                     functions                     //
//---------------------------------------------------//

void __libnx_initheap(void)
{
	static u8 inner_heap[INNER_HEAP_SIZE];
	extern void *fake_heap_start;
	extern void *fake_heap_end;

	// Configure the newlib heap.
	fake_heap_start = inner_heap;
	fake_heap_end = inner_heap + sizeof(inner_heap);
}

void __appInit(void)
{
	Result	rc;

	rc = smInitialize();
	if (R_FAILED(rc))
		diagAbortWithResult(MAKERESULT(Module_Libnx, LibnxError_InitFail_SM));

	rc = setsysInitialize();
	if (R_SUCCEEDED(rc))
	{
		SetSysFirmwareVersion fw;
		rc = setsysGetFirmwareVersion(&fw);
		if (R_SUCCEEDED(rc))
			hosversionSet(MAKEHOSVERSION(fw.major, fw.minor, fw.micro));
		setsysExit();
	}

	rc = hidInitialize();
	if (R_FAILED(rc))
		diagAbortWithResult(MAKERESULT(Module_Libnx, LibnxError_InitFail_HID));

	rc = hiddbgInitialize();
	if (R_FAILED(rc))
	{
		// printf("hiddbgInitialize(): 0x%x\n", rc)
	}
	if (R_SUCCEEDED(rc))
	{
		workmem = aligned_alloc(0x1000, workmem_size);
		if (workmem)
			init_flag = 1;
		else
		{
			// printf("workmem alloc failed\n");
		}
	}

	rc = socketInitialize(&sockInitConf);
	// if (R_FAILED(rc))
	// 	fatalThrow(rc);

	fsInitialize();
	fsdevMountSdmc();

	smExit();
}

void __appExit(void)
{
	hidExit();
	hiddbgExit();
	socketExit();
	fsdevUnmountAll();
	fsExit();
	free(workmem);
	init_flag = 0;
}

int main(int argc, char *argv[])
{
	socklen_t	len = sizeof(cliaddr);

	// FILE *arq = fopen("log.log", "w");

	if (init_flag == 0 || init_all() != 0)
		return (1);

	while (appletMainLoop())
	{
		int n;
		int message;

		n = recvfrom(socket_fd, &message, 4, 0, (struct sockaddr *)&cliaddr, &len);

		// if (message == HidDebugPadButton_A)
		// {
			controller_state.buttons |= HidDebugPadButton_A;
			hiddbgSetHdlsState(controller_handle, &controller_state);
			// printf("hiddbgSetHdlsState(): 0x%x\n", rc_set_controller_state);
			svcSleepThread(20 * 1000000);
			controller_state.buttons = 0;
			hiddbgSetHdlsState(controller_handle, &controller_state);
			// printf("hiddbgSetHdlsState(): 0x%x\n", rc_set_controller_state);
		// }
		// else if (message == 'b')
		// 	break ;

		break ;

		// svcSleepThread(5000 * 1e+6L);
	}

	finalize();
	// fclose(arq);

	return (0);
}
