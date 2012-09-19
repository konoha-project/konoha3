/* written by shinpei nakata */

import("konoha.bytes");

void test () {
	 assert("hoge".asBytes().encodeTo("UTF-8").asString() == "hoge");
	 assert("ほげほげ".asBytes().encodeTo("UTF-8").asString() == "ほげほげ");
}

test();
