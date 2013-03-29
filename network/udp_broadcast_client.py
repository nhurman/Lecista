import socket
import sys

def main() :
	sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
	sock.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST,1)
	sock.sendto("Hello from another subnet :)", ('10.133.8.255' ,50400))
	sock.close()

main()
