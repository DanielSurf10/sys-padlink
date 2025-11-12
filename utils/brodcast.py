import socket

BROADCAST_PORT = 6000
RESPONSE_PORT = 6000
BUFFER_SIZE = 1024

# Cria um socket UDP para escutar broadcasts
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
sock.bind(('', BROADCAST_PORT))

print(f"Escutando broadcasts na porta {BROADCAST_PORT}...")

# while True:
data, addr = sock.recvfrom(BUFFER_SIZE)
print(f"Recebido de {addr}: {data.decode()}")

# Responde ao remetente
response = "Sinal recebido!"
sock.sendto(response.encode(), (addr[0], RESPONSE_PORT))
print(f"Resposta enviada para {addr[0]}:{RESPONSE_PORT}")
