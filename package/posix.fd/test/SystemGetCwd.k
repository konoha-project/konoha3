//Written by Joseph

K.import("posix.fd");
K.import("konoha.string");

void test() {
	String str = System.getCwd();
	assert(str.get(0) == "/");
	assert(str.get(1) == "h");
	assert(str.get(2) == "o");
	assert(str.get(3) == "m");
	assert(str.get(4) == "e");
}

test();
