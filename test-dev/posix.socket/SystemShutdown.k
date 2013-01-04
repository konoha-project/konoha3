//Written by Joseph

import("Type.Bytes");
import("posix.socket");

void test() {
	int fd = System.socket(PF_INET, SOCK_STREAM, 0);
	System.listen(fd, SOMAXCONN);
	//TODO: System.shutdow returns -1 when no
	//      connection is established on MaxOSX.
	System.shutdown(fd, SHUT_RDWR);
	System.close(fd) == 0;
}

test();
