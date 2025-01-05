#ifndef PROTOCOL_H
# define PROTOCOL_H

//----------------------------------------------------------------------------//
//                      defines                      //
//----------------------------------------------------------------------------//

# define MAGIC_NUMBER	0x3276

// includes
# include <3ds.h>
# include "buttons_enums.h"

//----------------------------------------------------------------------------//
//                       enums                       //
//----------------------------------------------------------------------------//

enum e_packet_type
{
	HAS_BUTTONS			= BIT(0),
	HAS_RIGHT_ANALOG	= BIT(1),
	HAS_LEFT_ANALOG		= BIT(2),
	// HAS_GYROSCOPE		= BIT(3),		// Talvez
	// HAS_ACCELEROMETER	= BIT(4),
};

//----------------------------------------------------------------------------//
//                      typedefs                     //
//----------------------------------------------------------------------------//

typedef struct payload	payload;
typedef struct packet	packet;

//----------------------------------------------------------------------------//
//                      structs                      //
//----------------------------------------------------------------------------//

struct __attribute__((__packed__)) payload
{
	u32	keys;
	s32	analog_right_x;
	s32	analog_right_y;
	s32	analog_left_x;
	s32	analog_left_y;
};

struct __attribute__((__packed__)) packet
{
	u16		magic_number;
	u16		packet_size;
	u8		packet_type;
	payload	payload;
};

// functions
payload	transform_input_for_switch(u32 keys, circlePosition circle_pad);

#endif
