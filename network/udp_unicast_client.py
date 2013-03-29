import socket
import sys

def main() :
	sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
	sock.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST,1)
	sock.sendto("Unicast", ('10.133.8.34' ,50400))
	sock.close()

main()
