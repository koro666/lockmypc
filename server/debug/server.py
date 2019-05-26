#!/usr/bin/env python3.7
import sys
import time
import hashlib
import socket

def handle(address, port, secret, packet):
    print("[{}:{}] Packet: {}".format(address, port, packet.hex()))

    if len(packet) != 40:
        print('\x1b[91mFAILED\x1b[0m: Packet too short.')
        return False

    if packet[0:4] != b'LOCK':
        print('\x1b[91mFAILED\x1b[0m: Bad signature.')
        return False

    now = int(time.time())
    then_bytes = packet[4:8]
    then = int.from_bytes(then_bytes, 'big', signed=False)

    if abs(then - now) >= 60:
        print('\x1b[91mFAILED\x1b[0m: Time mismatch.')
        return False

    secret_bytes = secret.encode('utf-8')
    hash_bytes = hashlib.sha256(then_bytes + secret_bytes).digest()

    if packet[8:] != hash_bytes:
        print('\x1b[91mFAILED\x1b[0m: Secret mismatch.')
        return False

    print('\x1b[92mPASSED\x1b[0m')
    return True

def serve(address, port, secret):
    with socket.socket(socket.AF_INET6 if ':' in address else socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP) as sock:
        sock.bind((address, port))
        while True:
            packet, source = sock.recvfrom(40);
            handle(source[0], source[1], secret, packet)

def main(argv):
	address = argv[0] if len(argv) >= 1 else '127.0.0.1'
	port = int(argv[1]) if len(argv) >= 2 else 43666
	secret = argv[2] if len(argv) >= 3 else 'default'

	serve(address, port, secret)

if __name__ == '__main__':
	main(sys.argv[1:])
