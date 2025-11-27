import struct
import zlib
import socket
import time
import math

# Configurações do Protocolo
MAGIC_WORD	= 0xC0DE
VERSION		= 1
HEADER_FMT	= '>HBB L H H L' # Big Endian: Magic, Ver, Op, Sess, Seq, Len, CRC
HEADER_SIZE	= 16

# Opcodes e Erros
OP_DISCOVERY_REQ	= 0x01
OP_DISCOVERY_RES	= 0x02
OP_CONNECT_REQ		= 0x03
OP_CONNECT_ACK		= 0x04
OP_DATA_FRAME		= 0x05		# Não implementado
OP_DISCONNECT		= 0x07
OP_ERROR			= 0x99

CODE_SERVER_FULL		= 0x01
CODE_INVALID_SESSION	= 0x02
CODE_BAD_REQUEST		= 0x03

# Tamanho dos Payloads
PAYLOAD_SIZES = {
	OP_DISCOVERY_REQ: 0,
	OP_DISCOVERY_RES: 4,
	OP_CONNECT_REQ: 0,
	OP_CONNECT_ACK: 0,
	OP_DATA_FRAME: 20,
	OP_DISCONNECT: 0,
	OP_ERROR: 4,
}


# Exceções do Protocolo

class ProtocolError(Exception):
	"""Exceção base para todos os erros de protocolo UCP."""
	pass

class ProtocolDiscoveryError(ProtocolError):
	"""Erro durante a fase de descoberta do servidor."""
	pass

class ProtocolConnectionError(ProtocolError):
	"""Erro durante a fase de handshake/conexão."""
	pass

class ServerFullError(ProtocolError):
	"""Servidor está cheio e não aceita novas conexões."""
	pass

class InvalidSessionError(ProtocolError):
	"""Session ID inválido ou expirado."""
	pass

class BadRequestError(ProtocolError):
	"""Requisição malformada ou inválida."""
	pass

class ChecksumError(ProtocolError):
	"""Erro de validação CRC32."""
	pass


class UCPPacket:
	"""
	Representa um pacote do protocolo UCP (UDP Custom Protocol).

	Attributes:
		opcode (int): Código da operação (OP_DISCOVERY_REQ, OP_CONNECT_REQ, etc).
		session_id (int): ID da sessão (atribuído pelo servidor).
		sequence (int): Número de sequência do pacote.
		payload (bytes): Dados do payload.
		crc (int): Checksum CRC32 calculado.
	"""

	def __init__(self, opcode, session_id=0, sequence=0, payload=b''):
		"""
		Inicializa um novo pacote UCP.

		Args:
			opcode (int): Código da operação.
			session_id (int, optional): ID da sessão. Defaults to 0.
			sequence (int, optional): Número de sequência. Defaults to 0.
			payload (bytes, optional): Dados do payload. Defaults to b''.
		"""

		self.opcode = opcode
		self.session_id = session_id
		self.sequence = sequence
		self.payload = payload
		self.crc = 0 # Será calculado no pack()

	@staticmethod
	def _check_payload_size(op_code, length):
		"""
		Valida o tamanho do payload esperado para cada opcode.

		Args:
			op_code (int): Código da operação.
			length (int): Tamanho do payload recebido.

		Returns:
			bool: True se o tamanho é válido, False caso contrário.
		"""

		expected_size = PAYLOAD_SIZES.get(op_code, None)

		# Opcode desconhecido
		if expected_size is None:
			return False

		return expected_size == length

	@classmethod
	def from_bytes(cls, data):
		"""
		Factory method: Cria uma instância de UCPPacket a partir de bytes brutos.
		Realiza validação de Magic Word e CRC32.

		Args:
			data (bytes): Dados brutos recebidos da rede.

		Returns:
			UCPPacket: Instância do pacote decodificado.

		Raises:
			BadRequestError: Pacote menor que o tamanho mínimo ou payload inválido.
			ValueError: Magic Word inválida.
			ChecksumError: CRC32 não corresponde ao esperado.
		"""

		if len(data) < HEADER_SIZE:
			raise BadRequestError("Pacote menor que o cabeçalho mínimo.")

		# 1. Desempacota Header
		header = data[:HEADER_SIZE]
		payload = data[HEADER_SIZE:]
		magic, version, op_code, session_id, sequence, length, crc_recv = struct.unpack(HEADER_FMT, header)

		#####################
		# Fazer a validação em outra função

		# 2. Valida Magic
		if magic != MAGIC_WORD:
			raise ValueError(f"Magic Word inválida: 0x{magic:04X}")

		# 3. Valida CRC32
		# 3.1 Recria o buffer para cálculo do CRC (com campo CRC zerado)
		header_without_crc = struct.pack(HEADER_FMT, magic, version, op_code, session_id, sequence, length, 0)
		full_buffer = header_without_crc + payload

		# 3.2 Calcula o CRC32
		crc_calc = zlib.crc32(full_buffer) & 0xFFFFFFFF

		# 3.3 Compara os valores
		if crc_calc != crc_recv:
			raise ChecksumError(f"CRC falhou. Recebido: {crc_recv:X}, Calculado: {crc_calc:X}")

		# 4. Valida tamanho do payload
		if len(payload) != length:
			raise BadRequestError("Tamanho do payload não corresponde ao especificado no header.")

		# 5. Valida tamanho do payload específico do opcode
		if UCPPacket._check_payload_size(op_code, length) is False:
			raise BadRequestError("Tamanho do payload inválido para o opcode.")

		# 6. Retorna o objeto pronto
		return cls(op_code, session_id, sequence, payload)

	def pack(self):
		"""
		Serializa o objeto para bytes prontos para envio.
		Calcula automaticamente o CRC32 do pacote.

		Returns:
			bytes: Pacote completo (header + payload) serializado.
		"""

		# 1. Calcula tamanho do payload
		payload_len = len(self.payload)

		# 2. Header sem CRC (para cálculo)
		header_whitout_crc = struct.pack(HEADER_FMT,
			MAGIC_WORD, VERSION, self.opcode,
			self.session_id, self.sequence, payload_len, 0
		)

		# 3. Calcula CRC32
		self.crc = zlib.crc32(header_whitout_crc + self.payload) & 0xFFFFFFFF

		# 4. Header final com CRC calculado
		header_final = struct.pack(HEADER_FMT,
			MAGIC_WORD, VERSION, self.opcode,
			self.session_id, self.sequence, payload_len, self.crc
		)

		# 5. Retorna Header + Payload
		return header_final + self.payload

	# --- Helpers para Payloads Específicos ---

	@staticmethod
	def create_joypad_payload(keys, rx, ry, lx, ly):
		"""
		Cria o payload binário para envio de dados do joystick.

		Args:
			keys (int): Bitmask de botões pressionados (uint32).
			rx (int): Eixo X do analógico direito (-32768 a 32767).
			ry (int): Eixo Y do analógico direito (-32768 a 32767).
			lx (int): Eixo X do analógico esquerdo (-32768 a 32767).
			ly (int): Eixo Y do analógico esquerdo (-32768 a 32767).

		Returns:
			bytes: Payload de 20 bytes no formato Big Endian.
		"""

		# > = Big Endian, L = u32, i = s32
		return struct.pack('>Liiii', keys, rx, ry, lx, ly)

	def parse_error_payload(self):
		"""
		Decodifica o payload de um pacote de erro.

		Returns:
			tuple: (error_code, original_opcode) onde error_code é o código do erro
				   e original_opcode é o opcode que causou o erro.
		"""

		if len(self.payload) < 2:
			return (CODE_BAD_REQUEST, 0)
		return struct.unpack('>BBxx', self.payload[:4]) # xx = pular padding

	def parse_discovery_response(self):
		"""
		Decodifica o payload de resposta de descoberta.

		Returns:
			int: Número da porta UDP do servidor.
		"""

		return struct.unpack('>Hxx', self.payload[:4])[0]

	def parse(self, op_code):
		...

	def raise_error(self):
		"""
		Lança a exceção apropriada com base no payload de erro.

		Raises:
			ServerFullError: Código CODE_SERVER_FULL.
			InvalidSessionError: Código CODE_INVALID_SESSION.
			BadRequestError: Código CODE_BAD_REQUEST.
		"""

		code, original_opcode = self.parse_error_payload()

		if code == CODE_SERVER_FULL:
			raise ServerFullError(f"Servidor está cheio. Opcode original: {original_opcode}")
		if code == CODE_INVALID_SESSION:
			raise InvalidSessionError(f"Sessão inválida. Opcode original: {original_opcode}")
		if code == CODE_BAD_REQUEST:
			raise BadRequestError(f"Requisição inválida. Opcode original: {original_opcode}")


class UCPClient:
	"""
	Cliente UDP para o protocolo UCP.
	Gerencia descoberta, conexão e envio de dados de gamepad.

	Attributes:
		is_running (bool): Flag de controle do loop principal.
		sock (socket.socket): Socket UDP para comunicação.
		server_addr (tuple): Tupla (IP, porta) do servidor.
		session_id (int): ID da sessão atribuído pelo servidor.
		sequence_counter (int): Contador de sequência de pacotes.
	"""

	def __init__(self):
		"""
		Inicializa o cliente UCP.
		Configura o socket UDP com suporte a broadcast.
		"""

		self.is_running = True

		self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
		self.sock.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
		self.sock.settimeout(2.0)

		self.server_addr = None
		self.session_id = 0
		self.sequence_counter = 0

	def _send(self, packet: UCPPacket):
		"""
		Envia um pacote UCP para o servidor.
		Preenche automaticamente session_id e sequence.

		Args:
			packet (UCPPacket): Pacote a ser enviado.

		Raises:
			ProtocolConnectionError: Endereço do servidor não definido.
		"""

		# 1. Verifica se o endereço do servidor está definido
		if self.server_addr is None:
			raise ProtocolConnectionError("IP do servidor não definido.")

		# 2. Define o alvo
		target = self.server_addr

		# 3. Preenche Session ID e Sequence
		packet.session_id = self.session_id
		packet.sequence = self.sequence_counter

		# 4. Serializa o pacote
		data = packet.pack()

		# 5. Envia os dados
		self.sock.sendto(data, target)

		# 6. Incrementa o contador de sequência (wrap-around em 65535)
		self.sequence_counter = (self.sequence_counter + 1) % 65535

	def _recv(self):
		"""
		Recebe e converte bytes em objeto UCPPacket.
		Valida automaticamente Magic Word e CRC32.

		Returns:
			tuple: (UCPPacket, endereco) onde endereco é uma tupla (IP, porta).

		Raises:
			BadRequestError: Pacote malformado.
			ValueError: Magic Word inválida.
			ChecksumError: CRC32 inválido.
			TimeoutError: Timeout na recepção.
			BlockingIOError: Socket em modo non-blocking sem dados disponíveis.
		"""

		data, addr = self.sock.recvfrom(1024)
		packet = UCPPacket.from_bytes(data) # A classe valida Magic e CRC aqui

		return packet, addr

	def discover(self):
		"""
		Descobre o servidor via broadcast UDP.
		Envia OP_DISCOVERY_REQ e aguarda OP_DISCOVERY_RES.

		Raises:
			ProtocolDiscoveryError: Timeout, CRC inválido ou resposta malformada.

		Updates:
			self.server_addr: Tupla (IP, porta) do servidor encontrado.
		"""

		# print("[Discovery] Enviando Broadcast...")

		# Garantir que o socket está em non-blocking mode
		self.sock.setblocking(True)
		self.sock.settimeout(2.0)

		# Criação limpa do pacote
		packet = UCPPacket(OP_DISCOVERY_REQ)

		# Temporarily set server address for broadcast
		self.server_addr = ('<broadcast>', 9000)
		self._send(packet)
		self.server_addr = None

		try:
			packet_response, addr = self._recv()
		except (TimeoutError, ValueError, ChecksumError, BadRequestError) as e:
			raise ProtocolDiscoveryError(f"Erro durante a descoberta: {type(e).__name__}: {e}")

		if packet_response and packet_response.opcode == OP_DISCOVERY_RES:
			# print(f"[Discovery] Servidor em {self.server_addr}")
			port = packet_response.parse_discovery_response()
			self.server_addr = (addr[0], port)

	def connect(self):
		"""
		Estabelece conexão com o servidor via handshake.
		Envia OP_CONNECT_REQ e aguarda OP_CONNECT_ACK.

		Raises:
			ServerFullError: Servidor cheio.
			InvalidSessionError: Sessão inválida.
			ProtocolConnectionError: Erro de comunicação (timeout/CRC).

		Updates:
			self.session_id: ID da sessão atribuído pelo servidor.
		"""

		# Garantir que o socket está em non-blocking mode
		self.sock.setblocking(True)
		self.sock.settimeout(2.0)

		self.session_id = 0
		packet = UCPPacket(OP_CONNECT_REQ)
		self._send(packet)

		try:
			packet_response, _ = self._recv()

			if packet_response and packet_response.opcode == OP_ERROR:
				packet_response.raise_error()

		except (ServerFullError, InvalidSessionError):
			# Erros de protocolo: propaga sem conversão
			raise

		except (TimeoutError, ValueError, ChecksumError, BadRequestError) as e:
			raise ProtocolConnectionError(f"Erro durante o handshake: {type(e).__name__}: {e}")

		if packet_response and packet_response.opcode == OP_CONNECT_ACK:
			# print(f"[Handshake] Sucesso! ID: 0x{self.session_id:08X}")
			self.session_id = packet_response.session_id

	def run_game_loop(self):
		"""
		Loop principal de envio de inputs para o servidor (60 FPS).
		Envia pacotes OP_DATA_FRAME continuamente e drena respostas do servidor.

		Raises:
			ServerFullError: Servidor cheio durante o jogo.
			InvalidSessionError: Sessão invalidada pelo servidor.
		"""

		print("--- Game Loop (CTRL+C para sair) ---")
		frame_time = 1.0 / 60.0 # Número mágico, colocar em outro lugar
		t = 0

		self.sock.setblocking(False)

		while True:
			start_time = time.time()

			# 1. Gera Input
			t += 0.1
			lx = int(math.sin(t) * 32000)
			ly = int(math.cos(t) * 32000)

			# 2. Cria Payload usando o Helper Estático
			payload_bytes = UCPPacket.create_joypad_payload(0x01, 0, 0, lx, ly)

			# 3. Cria e Envia Pacote
			packet = UCPPacket(OP_DATA_FRAME, payload=payload_bytes)
			self._send(packet)

			# 3. Recebe Respostas, trata os erros e drena o buffer
			try:
				while True:
					packet_recv, _ = self._recv()

					# Verifica se o pacote é um ERRO e lança exceção
					if packet_recv.opcode == OP_ERROR:
						packet_recv.raise_error()

			except (ServerFullError, InvalidSessionError):
				# Erros críticos: propaga para reconexão
				raise

			except BadRequestError:
				# Pacote de erro inválido: ignora e continua
				pass

			except (BlockingIOError, ValueError, ChecksumError):
				# Sem dados ou pacote corrompido: ignora e continua
				pass

			# 4. Timing
			elapsed = time.time() - start_time
			if frame_time > elapsed:
				time.sleep(frame_time - elapsed)

	def close(self):
		"""
		Encerra a conexão com o servidor.
		Envia pacote OP_DISCONNECT e fecha o socket.
		"""

		if self.server_addr:
			try:
				packet = UCPPacket(OP_DISCONNECT)
				self._send(packet)
			except:
				pass
		self.sock.close()
		self.is_running = False

	def run(self):
		"""
		Loop principal do cliente.
		Gerencia descoberta, conexão e jogo com reconexão automática.

		Fluxo:
			1. Descoberta (loop até encontrar servidor)
			2. Handshake (loop até conectar)
			3. Game Loop (até erro ou desconexão)
			4. Reconexão automática em caso de erro

		Tratamento de Erros:
			- ServerFullError: Aguarda 5s e tenta novamente
			- InvalidSessionError: Reconecta imediatamente
			- KeyboardInterrupt: Encerra gracefully
			- Outras exceções: Reconecta após 1s
		"""

		while self.is_running:
			try:

				# Estado 1: Descoberta (loop até achar servidor)
				while self.server_addr is None:
					try:
						self.discover()
					except ProtocolDiscoveryError:
						time.sleep(1)

				# Estado 2: Handshake (loop até conectar no servidor)
				while self.session_id == 0:
					try:
						self.connect()
					except ProtocolConnectionError:
						time.sleep(1)

				# Estado 3: Loop de Jogo
				self.run_game_loop()

			except ServerFullError:
				# print("Servidor Cheio. Aguardando...")
				time.sleep(5)

			except (InvalidSessionError, ProtocolConnectionError):
				# print("Sessão Inválida. Reconectando...")
				self.server_addr = None
				self.session_id = 0
				time.sleep(1)

			except KeyboardInterrupt:
				# print("\nEncerrado.")
				self.close()

			except Exception as e:
				# print(f"Erro inesperado: {type(e).__name__}: {e}")
				self.server_addr = None
				self.session_id = 0
				time.sleep(1)


if __name__ == "__main__":
	client = UCPClient()
	client.run()
