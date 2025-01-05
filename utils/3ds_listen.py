import socket
import struct

UDP_IP = "0.0.0.0"
UDP_PORT = 6000

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind((UDP_IP, UDP_PORT))

print(f"Listening on UDP port {UDP_PORT}")

while True:
	data, addr = sock.recvfrom(1024)
	magico, packet_size, type, \
	buttons, \
	analog_left_x, analog_left_y, \
	analog_right_x, analog_right_y = struct.unpack("<HHbIiiii", data)
	print(f"Magico: {magico}, packet_size: {packet_size}, type: {type}, Buttons: {buttons}, Analog Left: ({analog_left_x}, {analog_left_y}), Analog Right: ({analog_right_x}, {analog_right_y})")
