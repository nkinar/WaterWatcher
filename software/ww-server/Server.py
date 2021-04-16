import socket
from threading import Thread
from loguru import logger
from Processing import DoAllProcessing
from IPLookup import IPLookup
from Influx import Influx
from constants import *

"""
REFERENCES:
https://www.tutorialspoint.com/socket-programming-with-multi-threading-in-python
"""

class Server:

    def __init__(self, restict_ip=True):
        self.processing = DoAllProcessing()
        self.ipcheck = IPLookup()
        self.influx = Influx()
        self.restrict_ip = restict_ip

    def open_server(self):
        soc = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        soc.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        soc.settimeout(SOCKET_TIMEOUT)  # timeout
        soc.bind((HOST, SERVPORT))
        soc.listen(DEFAULT_CONNECTIONS)
        logger.debug('Listening for connections...')
        while True:
            connection, address = soc.accept()
            ip, port = str(address[0]), str(address[1])
            logger.debug("IP and port connection " + ip + ":" + port)
            if self.restrict_ip:
                try:
                    if not self.ipcheck.determine_allowed(ip):
                        soc.close()
                        continue
                except ValueError:
                    soc.close()
                    continue
            dip = self.ipcheck.get_info_ip(ip)
            logger.debug('Logged IP Info:' + str(dip))
            r = self.influx.insert(dip, IP_DATABASE, MEASUREMENT_TABLE_IP)
            logger.debug('influx response: ' + str(r))
            Thread(target=self.client_thread, args=(connection, ip, port)).start()  # start the thread to receive data

    def process_input(self, connection, input_str):
        logger.debug("received: " + input_str)
        if input_str.strip() == TEST_STR:   # TEST string
            connection.sendall(OK_RESP)     # acknowledge receive of the test string
            logger.debug('Received test string, responded with OK response')
            return
        rv = self.processing.do_all_processing(input_str)
        rv = True
        if rv:
            connection.sendall(OK_RESP)
        else:
            connection.sendall(ERR_RESP)

    def receive_input(self, connection, max_buffer_size):
        client_input = connection.recv(max_buffer_size)
        return client_input

    def client_thread(self, connection, ip, port, max_buffer_size=MAX_CHAR_LEN):
        # the buffer needs to be built up by characters, since data will not arrive as complete strings
        logger.debug('in the client thread')
        buffer = ""
        while True:
            client_input = self.receive_input(connection, max_buffer_size)
            if not client_input:  # empty string or connection has closed
                break
            buffer += client_input.decode()
            if len(buffer) > MAX_CHAR_LEN:
                break           # limit the max transfer of data
            if buffer.endswith(CR_STR):
                self.process_input(connection, buffer)
                buffer = ""
        logger.debug("Client has closed connection...")

    def run_server_loop(self):
        while True:
            try:
                self.open_server()
            except Exception as e:
                logger.debug('Exception: ' + str(e))
                pass


def main():
    server = Server()
    server.run_server_loop()


if __name__ == "__main__":
    main()
