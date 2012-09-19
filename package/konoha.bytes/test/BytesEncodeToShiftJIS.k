/* written by shinpei nakata */

import("konoha.bytes");

void test () {
	assert("hoge".asBytes().encodeTo("SHIFT_JIS").asString() == "hoge");
	System.p("ほげほげ".asBytes().encodeTo("SHIFT_JIS").asString());
}

test();
