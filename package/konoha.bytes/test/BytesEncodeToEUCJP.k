/* written by shinpei nakata */

import("konoha.bytes");

void test () {
	assert("hoge".toBytes().encodeTo("EUC-JP").toString() == "hoge");
	System.p("ほげほげ".toBytes().encodeTo("EUC-JP").toString());
}

test();
