/* written by shinpei nakata */

K.import("konoha.bytes");


void test() {
	Bytes b = "".asBytes();
	String s = b.decodeFrom("");
	assert(s == "");
}

test();
