//Written by Joseph

import("posix.fd");
import("konoha.string");

void test() {
	String str = System.getCwd();
	assert(str.getSize() > 0);
	assert(str.get(0) == "/");
}

test();
