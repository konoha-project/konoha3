/* written by shinpei nakata */

K.import("konoha.bytes");

void test () {
	 assert("hoge".asBytes().encodeTo("SHIFT_JIS").asString() == "hoge");
	 assert("ほげほげ".asBytes().encodeTo("SHIFT_JIS").asString() == "ほげほげ");
}

test();
