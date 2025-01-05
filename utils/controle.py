import socket
import time
from struct import pack
from typing import Tuple

'''
Código	Tipo C				Tipo Python		Tamanho (bytes)		Descrição
x		padding byte		none			1					Byte de preenchimento
c		char				bytes			1					Caractere
b		signed char			int				1					Inteiro com sinal
B		unsigned char		int				1					Inteiro sem sinal
?		_Bool				bool			1					Booleano
h		short				int				2					Inteiro curto com sinal
H		unsigned short		int				2					Inteiro curto sem sinal
i		int					int				4					Inteiro com sinal
I		unsigned int		int				4					Inteiro sem sinal
l		long				int				4					Longo com sinal
L		unsigned long		int				4					Longo sem sinal
q		long long			int				8					Longo longo com sinal
Q		unsigned long long	int				8					Longo longo sem sinal
f		float				float			4					Ponto flutuante
d		double				float			8					Ponto flutuante duplo
s		char[]				bytes			variável			String (sequência de bytes)
p		char[]				bytes			variável			String (sequência de bytes) com comprimento prefixado
P		void *				int				8					Ponteiro
'''

def sendCommand(s, content):
	# content += '\r\n'
	# a = s.sendto(content.encode(), ("192.168.15.105", 6000))
	a = s.sendto(pack("<i", content), ("192.168.15.105", 6000))
	print(a)

def get_packet(buttons: int, analog_left: Tuple[int, int], analog_right: Tuple[int, int],
			   desligar: bool = False) -> bytes:
	"""
	Generates a packet with the given button states and analog stick positions.

	Args:
		buttons (int): A number that in binary represents the states of the buttons.
		analog_left (tuple): A tuple containing the X and Y positions of the left analog stick.
		analog_right (tuple): A tuple containing the X and Y positions of the right analog stick.
		desligar (bool, optional): A flag to indicate if the device should be turned off. Defaults to False.

	Returns:
		bytes: A packed byte sequence representing the packet.
	"""
	magico = 0x3276
	if desligar:
		magico = 0
	packet = pack("<HHbIiiii", magico, 25, 0b111,
		# Keys
		buttons,
		# Analog Left
		# X Y
		analog_left[0], analog_left[1],
		# Analog Right
		# X Y
		analog_right[0], analog_right[1]
	)
	return packet


s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

press_a = get_packet(0b0001, (0, 0), (0, 0))
press_b = get_packet(0b0010, (0, 0), (0, 0))
press_ab = get_packet(0b0011, (0, 0), (0, 0))
release_all = get_packet(0b0, (0, 0), (0, 0))
desligar = get_packet(0, (0, 0), (0, 0), True)

# a = s.sendto(press_ab, ("192.168.15.105", 6000))
# time.sleep(2)
# a = s.sendto(release_all, ("192.168.15.105", 6000))
# a = s.sendto(desligar, ("192.168.15.105", 6000))

# for i in range(0, 16):
# 	button = get_packet((2 ** i))
# 	s.sendto(button, ("192.168.15.105", 6000))
# 	print(bin((2 ** i)))
# 	time.sleep(1)

# button = get_packet((2 ** 4))
# s.sendto(button, ("192.168.15.105", 6000))
# s.sendto(release_all, ("192.168.15.105", 6000))

for i in range(-32768, 32768, 100):
	button = get_packet(0, (i, i), (i, i))
	s.sendto(button, ("192.168.15.105", 6000))
	print(i)
	time.sleep(0.01)

button = get_packet(0, (0, 0), (0, 0))
s.sendto(button, ("192.168.15.105", 6000))
s.sendto(desligar, ("192.168.15.105", 6000))

s.sendto(get_packet(0, (0, 0), (0, 0), True), ("192.168.15.105", 6000))

s.close()
