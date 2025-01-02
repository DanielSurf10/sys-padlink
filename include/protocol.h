#ifndef PROTOCOL_H
# define PROTOCOL_H

// Defines

# define PORT 6000

// Includes

#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <switch.h>


// Variables
// Work memory
extern u8								*workmem;
extern size_t							workmem_size;

// Controller session and handle
extern HiddbgHdlsSessionId				session_id;
extern HiddbgHdlsHandle					controller_handle;

// Controller type and state
extern HidDeviceType					controller_initialized_type;
extern HiddbgHdlsState					controller_state;

// Controller device information
extern HiddbgHdlsDeviceInfo				controller_device;

// Socket
extern int								socket_fd;
extern struct sockaddr_in				servaddr;
extern struct sockaddr_in				cliaddr;


// Enums

enum e_packet_type
{
	HAS_BUTTONS			= BIT(0),
	HAS_RIGHT_ANALOG	= BIT(1),
	HAS_LEFT_ANALOG		= BIT(2),
	// HAS_GYROSCOPE		= BIT(3),		// Talvez
	// HAS_ACCELEROMETER	= BIT(4),
};

// Typedefs

typedef struct payload	payload;
typedef struct packet	packet;

// Structs

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

// Functions

Result	init_all(void);
void	finalize(void);

#endif
