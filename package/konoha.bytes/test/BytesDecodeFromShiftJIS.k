/* written by shinpei nakata */

import("konoha.bytes");

void test () {
	 assert("hoge".toBytes().encodeTo("SHIFT_JIS").decodeFrom("SHIFT_JIS") == "hoge");
	 assert("ほげほげ".toBytes().encodeTo("SHIFT_JIS").decodeFrom("SHIFT_JIS") == "ほげほげ");
}

test();
