import("konoha");
import("Type.Bytes");
import("konoha.socket");


void main () {
	String dstIP = "127.0.0.1";
	int dstPort = 10001;
	int dstFamily = AF_INET;
	SockAddr dstInfo = new SockAddr();

	String srcIP = "127.0.0.1";
	int srcPort = 10002;
	int srcFamily = AF_INET;

	int fd = socket(srcFamily, SOCK_STREAM, 0);
	int ret = bind(fd, "", srcPort, srcFamily);

	ret = connect(fd, dstIP, dstPort, dstFamily);

	Bytes msg = "Hello world!".toBytes();
	ret = sendto(fd, msg, 0, dstIP, dstPort, dstFamily);

	close(fd);

}

main();
