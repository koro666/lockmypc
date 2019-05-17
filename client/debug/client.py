#!/usr/bin/env python3.7
import sys
import time
import hashlib
import socket

def send(address, port, secret):
	now_bytes = int(time.time()).to_bytes(4, 'big', signed=False)
	secret_bytes = secret.encode('utf-8')
	hash_bytes = hashlib.sha256(now_bytes + secret_bytes).digest()

	packet = b'LOCK' + now_bytes + hash_bytes
	print("Packet: {}".format(packet.hex()))

	with socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP) as sock:
		sock.sendto(packet, 0, (address, port))

def main(argv):
	address = argv[0] if len(argv) >= 1 else '127.0.0.1'
	port = int(argv[1]) if len(argv) >= 2 else 1024
	secret = argv[2] if len(argv) >= 3 else 'default'

	send(address, port, secret)

if __name__ == '__main__':
	main(sys.argv[1:])
