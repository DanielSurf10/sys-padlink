#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "protocol.h"
#include "utils.h"

#define INNER_HEAP_SIZE 0x80000

//----------------------------------------------------------------------------//
//                      globals                      //
//----------------------------------------------------------------------------//

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

// File
#ifdef DEBUG
FILE		*arq;
#else
FILE		*arq = NULL;
#endif

//----------------------------------------------------------------------------//
//                     functions                     //
//----------------------------------------------------------------------------//

void __libnx_initheap(void)
{
	static u8 inner_heap[INNER_HEAP_SIZE];
	extern void *fake_heap_start;
	extern void *fake_heap_end;

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

#ifdef DEBUG
	fsInitialize();
	fsdevMountSdmc();
#endif

	smExit();
}

void __appExit(void)
{
	hidExit();
	hiddbgExit();
	socketExit();

#ifdef DEBUG
	fsdevUnmountAll();
	fsExit();
#endif

	free(workmem);
	init_flag = 0;
}

int main(int argc, char *argv[])
{
	socklen_t	len = sizeof(cliaddr);

	if (init_flag == 0 || init_all() != 0)
		return (1);

#ifdef DEBUG
	arq = fopen("log.log", "w");
#endif

	while (appletMainLoop())
	{
		unsigned int	n;
		packet			new_device_state;

		n = recvfrom(socket_fd, &new_device_state, sizeof(packet), MSG_WAITALL, (struct sockaddr *)&cliaddr, &len);

		// if (new_device_state.magic_number != MAGIC_NUMBER)
		// 	print_to_file(arq, "magic number error - %d\n", new_device_state.magic_number);
		// else if (new_device_state.packet_size != n)
		// 	print_to_file(arq, "invalid packet_size - %d\n", new_device_state.packet_size);
		// else
		// {
		// 	print_to_file(arq, "keys: %d\n", new_device_state.payload.keys);
		// 	print_to_file(arq, "analog right: %d %d \n",
		// 		new_device_state.payload.analog_right_x, new_device_state.payload.analog_right_y
		// 	);
		// 	print_to_file(arq, "analog left : %d %d \n",
		// 		new_device_state.payload.analog_left_x, new_device_state.payload.analog_left_y
		// 	);
		// 	print_to_file(arq, "A: %d - B: %d - X: %d - Y: %d\n",
		// 		(new_device_state.payload.keys & HidDebugPadButton_A) > 0,
		// 		(new_device_state.payload.keys & HidDebugPadButton_B) > 0,
		// 		(new_device_state.payload.keys & HidDebugPadButton_X) > 0,
		// 		(new_device_state.payload.keys & HidDebugPadButton_Y) > 0
		// 	);
		// }

		n = apply_device_state(n, new_device_state);

		if (n != 0)
			break ;


		// if (new_device_state == HidDebugPadButton_A)
		// {
			// controller_state.buttons = HidDebugPadButton_B;
			// hiddbgSetHdlsState(controller_handle, &controller_state);
			// svcSleepThread(20 * 1000000);
			// controller_state.buttons = 0;
			// hiddbgSetHdlsState(controller_handle, &controller_state);
			// svcSleepThread(20 * 1000000);
			// controller_state.buttons = HidDebugPadButton_A;
			// hiddbgSetHdlsState(controller_handle, &controller_state);
			// svcSleepThread(20 * 1000000);
			// controller_state.buttons = 0;
			// hiddbgSetHdlsState(controller_handle, &controller_state);
		// }
		// else if (new_device_state == 'b')
		// 	break ;

		// break ;

		svcSleepThread(5 * 1e+6L);
	}

	finalize();

#ifdef DEBUG
	fclose(arq);
#endif

	return (0);
}
