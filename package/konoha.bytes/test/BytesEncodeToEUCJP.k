/* written by shinpei nakata */

import("konoha.bytes");

void test () {
	assert("hoge".asBytes().encodeTo("EUC-JP").asString() == "hoge");
	System.p("ほげほげ".asBytes().encodeTo("EUC-JP").asString());
}

test();
