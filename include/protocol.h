#ifndef PROTOCOL_H
# define PROTOCOL_H

//---------------------------------------------------//
//                      defines                      //
//---------------------------------------------------//

# define PORT 6000

//---------------------------------------------------//
//                      includes                     //
//---------------------------------------------------//

#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <switch.h>

//---------------------------------------------------//
//                     variables                     //
//---------------------------------------------------//

// work memory
extern u8								*workmem;
extern size_t							workmem_size;

// controller session and handle
extern HiddbgHdlsSessionId				session_id;
extern HiddbgHdlsHandle					controller_handle;

// controller type and state
extern HidDeviceType					controller_initialized_type;
extern HiddbgHdlsState					controller_state;

// controller device information
extern HiddbgHdlsDeviceInfo				controller_device;

// socket
extern int								socket_fd;
extern struct sockaddr_in				servaddr;
extern struct sockaddr_in				cliaddr;

//---------------------------------------------------//
//                       enums                       //
//---------------------------------------------------//

enum e_packet_type
{
	HAS_BUTTONS			= BIT(0),
	HAS_RIGHT_ANALOG	= BIT(1),
	HAS_LEFT_ANALOG		= BIT(2),
	// HAS_GYROSCOPE		= BIT(3),		// Talvez
	// HAS_ACCELEROMETER	= BIT(4),
};

//---------------------------------------------------//
//                      typedefs                     //
//---------------------------------------------------//

typedef struct payload	payload;
typedef struct packet	packet;

//---------------------------------------------------//
//                      structs                      //
//---------------------------------------------------//

struct __attribute__((__packed__)) payload
{
	u16	keys;
	s32	analog_right_x;
	s32	analog_right_y;
	s32	analog_left_x;
	s32	analog_left_y;
};

struct __attribute__((__packed__)) packet
{
	u16		magic_number;
	u8		packet_type;
	u16		packet_size;
	payload	payload;
};

//---------------------------------------------------//
//                     functions                     //
//---------------------------------------------------//

Result	init_all(void);
void	finalize(void);

#endif
