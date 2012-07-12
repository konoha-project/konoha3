// written by shinpei nakata

void global_setup() {
	K.import("konoha.bytes");
}
global_setup();

boolean test_decode() {
	Bytes b = "".toBytes();
	String s = b.decodeFrom("");
	System.p(s);
}

test_decode();
