import socket
import struct

UDP_IP = "0.0.0.0"
UDP_PORT = 6000

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind((UDP_IP, UDP_PORT))

print(f"Listening on UDP port {UDP_PORT}")

while True:
	data, addr = sock.recvfrom(1024)
	unpacked_data = struct.unpack('<hh', data)
	print(f"Received message: {unpacked_data} from {addr}")
