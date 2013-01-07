//Written by Joseph

import("JavaScript.String");
import("posix.path");

void test() {
	String str = System.getCwd();
	assert(str.getSize() > 0);
	assert(str.get(0) == "/");
}

test();
