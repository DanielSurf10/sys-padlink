# salvar como gamepad_sender.py
import socket
import json
import time
import pygame
from struct import pack
from typing import Tuple


# Config
UDP_IP = "192.168.15.105"   # destino
UDP_PORT = 6000
SEND_INTERVAL = (1 / 60)  # 60 Hz

# Inicializa pygame e joystick
pygame.init()
pygame.joystick.init()

if pygame.joystick.get_count() == 0:
	print("Nenhum joystick detectado. Conecte um controle e rode novamente.")
	raise SystemExit

joy = pygame.joystick.Joystick(0)
joy.init()
print("Usando joystick:", joy.get_name())

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

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

def remap_buttons(buttons, hats, axes_triggers):
	buttons_remaped = [
		buttons[0],					# A
		buttons[1],					# B
		buttons[2],					# X
		buttons[3],					# Y
		buttons[9],					# L Stick
		buttons[10],				# R Stick
		buttons[4],					# L
		buttons[5],					# R
		axes_triggers[0],			# ZL
		axes_triggers[1],			# ZR
		buttons[7],					# Start  / +
		buttons[6],					# Select / -
		int(hats[0] == -1),			# <-
		int(hats[1] == 1),			# /\
		int(hats[0] == 1),			# ->
		int(hats[1] == -1),			# \/
	]

	return (buttons_remaped)


def read_state():
	pygame.event.pump()  # processa fila de eventos — necessário para atualizar estado

	# mapeia float [-1.0, 1.0] para int [-32768, 32768]
	def _map_axis_to_int(v: float) -> int:
		return int(v * 32768)

	# O axis do y (cima e baixo) ficam invertidos
	# Para mandar para o switch precisa estar certo
	# Direcional para cima deve estar positivo
	# Direcional para baixo deve estar negativo
	# O mapeamento de [-1.0, 1.0] para [-32768, 32768] é feito multiplicando por 32768
	axes_left  = (int(joy.get_axis(0) * 32768), int(joy.get_axis(1) * 32768 * -1))
	axes_right = (int(joy.get_axis(3) * 32768), int(joy.get_axis(4) * 32768 * -1))

	# Aqui o valor vai de -1.0 a 1.0
	# Para acionar ele precisar estar maior que -0.8, um pouco apertado
	axes_triggers = (
		1 if joy.get_axis(2) > -0.8 else 0,
		1 if joy.get_axis(5) > -0.8 else 0
	)

	buttons = [joy.get_button(i) for i in range(joy.get_numbuttons())]
	hats = joy.get_hat(0)

	# Mapeia os botões de xbox para os botões do Switch
	buttons_remaped = remap_buttons(buttons, hats, axes_triggers)

	# Convete os botões de lista para um único inteiro
	buttons_bin = 0
	for b in range(len(buttons_remaped)):
		buttons_bin = (buttons_bin << 1) | buttons_remaped[-1 - b]

	print(buttons_remaped, axes_left, axes_right)
	return get_packet(buttons_bin, axes_left, axes_right)

def send_state(state):
	sock.sendto(state, (UDP_IP, UDP_PORT))


if __name__ == "__main__":
	try:
		print("Enviando entradas para %s:%d (Ctrl-C para sair)" % (UDP_IP, UDP_PORT))
		while True:
			state = read_state()
			send_state(state)
			time.sleep(SEND_INTERVAL)

	except KeyboardInterrupt:
		print("Encerrando...")

	finally:
		joy.quit()
		pygame.quit()
		sock.sendto(get_packet(0, (0, 0), (0, 0), True), (UDP_IP, UDP_PORT))
		sock.close()
