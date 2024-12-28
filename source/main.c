#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <switch.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>

// Size of the inner heap (adjust as necessary).
#define INNER_HEAP_SIZE 0x80000

//---------------------------------------------------//
//                      Globals                      //
//---------------------------------------------------//

u32		__nx_applet_type = AppletType_None;
u32		__nx_fs_num_sessions = 1;

// Initialization flag
bool	init_flag = 0;

// Work memory
u8		*workmem = NULL;
size_t	workmem_size = 0x1000;

// Controller session and handle
HiddbgHdlsSessionId		session_id					= {0};
HiddbgHdlsHandle		controller_handle			= {0};

// Controller type and state
HidDeviceType			controller_initialized_type	= HidDeviceType_FullKey3;
HiddbgHdlsState			controller_state			= {0};

// Controller device information
HiddbgHdlsDeviceInfo	controller_device = {
	.deviceType				= HidDeviceType_FullKey3,			// Pro Controller
	.npadInterfaceType		= HidNpadInterfaceType_Bluetooth,
	.singleColorBody		= RGBA8_MAXALPHA(255,255,255),
	.singleColorButtons 	= RGBA8_MAXALPHA(0,0,0),
	.colorLeftGrip			= RGBA8_MAXALPHA(230,255,0),
	.colorRightGrip			= RGBA8_MAXALPHA(0,40,20),
	.pad					= {0}
};

// Socket
int					socket_fd = -1;
struct sockaddr_in	servaddr, cliaddr;
static const SocketInitConfig sockInitConf = {
	.tcp_tx_buf_size        = 0x200,
	.tcp_rx_buf_size        = 0x400,
	.tcp_tx_buf_max_size    = 0x400,
	.tcp_rx_buf_max_size    = 0x800,
	// We're not using tcp anyways

	.udp_tx_buf_size = 0x2600,
	.udp_rx_buf_size = 0xA700,

	.sb_efficiency = 2,

	.num_bsd_sessions = 3,
	.bsd_service_type = BsdServiceType_User
};

void __libnx_initheap(void)
{
	static u8 inner_heap[INNER_HEAP_SIZE];
	extern void* fake_heap_start;
	extern void* fake_heap_end;

	// Configure the newlib heap.
	fake_heap_start = inner_heap;
	fake_heap_end   = inner_heap + sizeof(inner_heap);
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

int main(int argc, char* argv[])
{
	Result	rc_buffer = 0;
	Result	rc_device = 0;
	Result	rc_all = 0;
	Result	rc_set_controller_state = 0;
	Result	rc_socket = 0;
	Result	rc_bind = 0;

	FILE *arq = fopen("log.log", "w");

	if (init_flag == 0)
		return (1);

	controller_state.battery_level = 4;
	rc_buffer = hiddbgAttachHdlsWorkBuffer(&session_id, workmem, workmem_size);
	// printf("hiddbgAttachHdlsWorkBuffer(): 0x%x\n", rc_buffer);
	if (R_SUCCEEDED(rc_buffer))
	{
		rc_device = hiddbgAttachHdlsVirtualDevice(&controller_handle, &controller_device);
		// printf("hiddbgAttachHdlsVirtualDevice(): 0x%x\n", rc_device);
	}

	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = INADDR_ANY;
	servaddr.sin_port = htons(6000);
	if ((socket_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
		rc_socket = 1;
	else if ((rc_bind = bind(socket_fd, (struct sockaddr *)&servaddr, sizeof(servaddr))) < 0)
		rc_socket = 1;

	// printf("Entrando no looping infinito\n");

	// svcSleepThread(5 * 1000000000L);
	rc_all = R_SUCCEEDED(rc_buffer) || R_SUCCEEDED(rc_device) || R_SUCCEEDED(rc_socket);

	while (appletMainLoop())
	{
		socklen_t	len = sizeof(cliaddr);
		int			n;
		int			message;

		n = recvfrom(socket_fd, &message, 4, 0, (struct sockaddr *)&cliaddr, &len);

		if (message == HidDebugPadButton_A)
		{
			controller_state.buttons |= message;
			rc_set_controller_state = hiddbgSetHdlsState(controller_handle, &controller_state);
			// printf("hiddbgSetHdlsState(): 0x%x\n", rc_set_controller_state);
			svcSleepThread(20 * 1000000);
			controller_state.buttons = 0;
			rc_set_controller_state = hiddbgSetHdlsState(controller_handle, &controller_state);
			// printf("hiddbgSetHdlsState(): 0x%x\n", rc_set_controller_state);
		}
		else if (message == 'b')
			break ;

		break;

		// svcSleepThread(5000 * 1e+6L);
	}

	// printf("Saindo no looping infinito\n");

	if (R_SUCCEEDED(rc_device))
		hiddbgDetachHdlsVirtualDevice(controller_handle);
	if (R_SUCCEEDED(rc_buffer))
		hiddbgReleaseHdlsWorkBuffer(session_id);

	fclose(arq);
	close(socket_fd);

	return (0);
}
