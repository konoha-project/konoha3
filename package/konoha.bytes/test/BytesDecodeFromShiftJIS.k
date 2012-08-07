/* written by shinpei nakata */

K.import("konoha.bytes");

void test () {
	 //p("'" + "ほげほげ".asBytes().encodeTo("SHIFT_JIS").decodeFrom("SHIFT_JIS") + "'");
	 assert("ほげほげ".asBytes().encodeTo("SHIFT_JIS").decodeFrom("SHIFT_JIS") == "ほげほげ");
}

test();
