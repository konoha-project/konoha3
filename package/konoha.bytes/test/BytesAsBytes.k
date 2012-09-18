/* written by shinpei nakata */

K.import("konoha.bytes");
K.import("konoha.null");

void test() {
	Bytes b = "".asBytes();
	String s = b.decodeFrom("");
	assert(s == null);
}

test();
