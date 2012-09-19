// written by shinpei
import("konoha.bytes");
import("konoha.socket");
int makeSocket() {
  return System.socket(AF_INET, SOCK_STREAM, 0);
}

boolean makeServerSocket(int servPort) {
	int fd = makeSocket();
	System.setsockopt(fd, SO_REUSEADDR, 1);
	int ret = System.bind(fd, "hoge", servPort, AF_INET);
	return true;
}

System.p(makeServerSocket(9999));
