#include "protocol.h"

static int map(int value, int value_3ds_low, int value_3ds_high, int value_switch_low, int value_switch_high)
{
	return (value - value_3ds_low) * (value_switch_high - value_switch_low) / (value_3ds_high - value_3ds_low) + value_switch_low;
}

payload	transform_input_for_switch(u32 keys, circlePosition circle_pad)
{
	payload	transformed_input;

	transformed_input.analog_right_x = map(circle_pad.dx, -156, 156, -32767, 32767);
	transformed_input.analog_right_y = map(circle_pad.dy, -156, 156, -32767, 32767);
	transformed_input.analog_left_x = 0;
	transformed_input.analog_left_y = 0;

	transformed_input.keys  = SW_BUTTON_A(keys & KEY_A);
	transformed_input.keys |= SW_BUTTON_B(keys & KEY_B);
	transformed_input.keys |= SW_BUTTON_SELECT(keys & KEY_SELECT);
	transformed_input.keys |= SW_BUTTON_START(keys & KEY_START);
	transformed_input.keys |= SW_BUTTON_DPAD_RIGHT(keys & KEY_DRIGHT);
	transformed_input.keys |= SW_BUTTON_DPAD_LEFT(keys & KEY_DLEFT);
	transformed_input.keys |= SW_BUTTON_DPAD_UP(keys & KEY_DUP);
	transformed_input.keys |= SW_BUTTON_DOWN(keys & KEY_DDOWN);
	transformed_input.keys |= SW_BUTTON_ZR(keys & KEY_R);
	transformed_input.keys |= SW_BUTTON_ZL(keys & KEY_L);
	transformed_input.keys |= SW_BUTTON_X(keys & KEY_X);
	transformed_input.keys |= SW_BUTTON_Y(keys & KEY_Y);

	return (transformed_input);
}
