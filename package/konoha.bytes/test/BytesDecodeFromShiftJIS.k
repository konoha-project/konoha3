/* written by shinpei nakata */

K.import("konoha.bytes");

void test () {
	 assert("hoge".asBytes().encodeTo("SHIFT_JIS").decodeFrom("SHIFT_JIS") == "hoge");
	 assert("ほげほげ".asBytes().encodeTo("SHIFT_JIS").decodeFrom("SHIFT_JIS") == "ほげほげ");
}

test();
