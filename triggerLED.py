#!/usr/bin/python
import os
import sys
import socket
import threading
import SocketServer
import logging
from time import sleep


# - - - - - - - - - - - - - - - - 
# - - SOCKET CLIENT FUNCTION  - -
# - - - - - - - - - - - - - - - -
def client(ip, port, message):
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.connect((ip, port))
    logging.basicConfig(filename='test.log', level=logging.DEBUG)
    logging.error(message)
    try:
        sock.sendall(message)
        # wait for the python script to produce and send all the data
        sleep(0.3)
        response = sock.recv(1048576)
        print "Content-type: text/html \n\n"
        print response
    finally:
        sock.close()


color = sys.argv[1]
client('localhost', 7777, "color=" + color + "&mode=0&repeat=0&period=1000&json=0&remote_addr=127.0.0.1")


