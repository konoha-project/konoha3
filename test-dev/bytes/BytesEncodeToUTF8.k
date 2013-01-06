/* written by shinpei nakata */

import("Type.Bytes");

void test () {
	 assert("hoge".toBytes().encodeTo("UTF-8").toString() == "hoge");
	 assert("ほげほげ".toBytes().encodeTo("UTF-8").toString() == "ほげほげ");
}

test();
