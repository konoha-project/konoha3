//Written by Joseph

K.import("konoha.new");
K.import("konoha.bytes");
K.import("posix.fd");

void test() {
	Bytes buf = new Bytes(4096);
	assert(System.readlink("symlinked.txt", buf.asString(), buf.getsize()) > 0);
	assert(System.readlink("notexist.txt", buf.asString(), buf.getsize()) == -1);
}

test();
