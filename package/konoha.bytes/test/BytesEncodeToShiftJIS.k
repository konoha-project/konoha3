/* written by shinpei nakata */

import("konoha.bytes");

void test () {
	assert("hoge".toBytes().encodeTo("SHIFT_JIS").toString() == "hoge");
	System.p("ほげほげ".toBytes().encodeTo("SHIFT_JIS").toString());
}

test();
