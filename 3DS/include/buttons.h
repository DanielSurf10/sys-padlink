#ifndef SWITCH_BUTTONS_H
# define SWITCH_BUTTONS_H

// defines
# define CONVERT_TO_BOOL(button)		((button) ? 1 : 0)

# define SW_BUTTON_A(button)			(CONVERT_TO_BOOL(button) << (  0 ))
# define SW_BUTTON_B(button)			(CONVERT_TO_BOOL(button) << (  1 ))
# define SW_BUTTON_SELECT(button)		(CONVERT_TO_BOOL(button) << ( 11 ))
# define SW_BUTTON_START(button)		(CONVERT_TO_BOOL(button) << ( 10 ))
# define SW_BUTTON_DPAD_RIGHT(button)	(CONVERT_TO_BOOL(button) << ( 14 ))
# define SW_BUTTON_DPAD_LEFT(button)	(CONVERT_TO_BOOL(button) << ( 12 ))
# define SW_BUTTON_DPAD_UP(button)		(CONVERT_TO_BOOL(button) << ( 13 ))
# define SW_BUTTON_DOWN(button)			(CONVERT_TO_BOOL(button) << ( 15 ))
# define SW_BUTTON_ZR(button)			(CONVERT_TO_BOOL(button) << (  9 ))
# define SW_BUTTON_ZL(button)			(CONVERT_TO_BOOL(button) << (  8 ))
# define SW_BUTTON_X(button)			(CONVERT_TO_BOOL(button) << (  2 ))
# define SW_BUTTON_Y(button)			(CONVERT_TO_BOOL(button) << (  3 ))

// Talvez algum dia, mas precisa mudar o ZL e ZR
// # define SW_BUTTON_L(button)			(CONVERT_TO_BOOL(button) << (  6 ))
// # define SW_BUTTON_R(button)			(CONVERT_TO_BOOL(button) << (  7 ))

// enums
enum switch_buttons
{
	SW_A				= (1UL << (  0 )),
	SW_B				= (1UL << (  1 )),
	SW_X				= (1UL << (  2 )),
	SW_Y				= (1UL << (  3 )),
	SW_STICK_L			= (1UL << (  4 )),
	SW_STICK_R			= (1UL << (  5 )),
	SW_L				= (1UL << (  6 )),
	SW_R				= (1UL << (  7 )),
	SW_ZL				= (1UL << (  8 )),
	SW_ZR				= (1UL << (  9 )),
	SW_PLUS				= (1UL << ( 10 )),
	SW_MINUS			= (1UL << ( 11 )),
	SW_DPAD_LEFT		= (1UL << ( 12 )),
	SW_DPAD_UP			= (1UL << ( 13 )),
	SW_DPAD_RIGHT		= (1UL << ( 14 )),
	SW_DPAD_DOWN		= (1UL << ( 15 ))
};

enum _3ds_buttons {
	_3DS_A				= (1UL << (  0 )),
	_3DS_B				= (1UL << (  1 )),
	_3DS_SELECT			= (1UL << (  2 )),
	_3DS_START			= (1UL << (  3 )),
	_3DS_DPAD_RIGHT		= (1UL << (  4 )),
	_3DS_DPAD_LEFT		= (1UL << (  5 )),
	_3DS_DPAD_UP		= (1UL << (  6 )),
	_3DS_DPAD_DOWN		= (1UL << (  7 )),
	_3DS_R				= (1UL << (  8 )),
	_3DS_L				= (1UL << (  9 )),
	_3DS_X				= (1UL << ( 10 )),
	_3DS_Y				= (1UL << ( 11 )),
	// _3DS_ZL				= (1UL << ( 14 )),	// Talvez algum dia
	// _3DS_ZR				= (1UL << ( 15 ))
};

#endif
