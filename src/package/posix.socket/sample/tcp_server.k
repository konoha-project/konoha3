import("konoha");
import("Type.Bytes");
import("konoha.socket");


void main () {
	String dstIP = "127.0.0.1";
	int dstPort = 10002;
	int dstFamily = AF_INET;
	SockAddr dstInfo = new SockAddr();

	String srcIP = "127.0.0.1";
	int srcPort = 10001;
	int srcFamily = AF_INET;

	int fd = socket(srcFamily, SOCK_STREAM, 0);
	int ret = bind(fd, "", srcPort, srcFamily);
	ret = listen(fd, 1);

	int dstFd = accept(fd, dstInfo);

	int[] rfd = new int[2];
	int[] wfd = new int[2];
	int[] efd = new int[2];
	ret = select(rfd, wfd, efd, 1, 500000);

	Bytes buf = new Bytes(32);
	ret = recv(dstFd, buf, 0);
	p(buf.asString());
	close(dstFd);
	close(fd);
}

main();
