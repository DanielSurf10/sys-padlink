import socket
import time
from struct import pack

def sendCommand(s, content):
	# content += '\r\n'
	# a = s.sendto(content.encode(), ("192.168.15.105", 6000))
	a = s.sendto(pack("<i", content), ("192.168.15.105", 6000))
	print(a)

s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

# sendCommand(s, 'a')
sendCommand(s, 2)

s.close()
