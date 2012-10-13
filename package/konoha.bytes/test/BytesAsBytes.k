/* written by shinpei nakata */

import("konoha.bytes");
import("konoha.null");

void test() {
	Bytes b = "".toBytes();
	String s = b.decodeFrom("");
	assert(s == null);
}

test();
