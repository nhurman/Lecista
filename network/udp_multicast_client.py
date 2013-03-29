import socket
import struct
import time

IP="224.0.0.150"

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
sock.setsockopt(socket.IPPROTO_IP, socket.IP_MULTICAST_TTL, 4)

sock.sendto("Multicast", (IP, 50400))
