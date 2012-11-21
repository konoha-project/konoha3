/* written by shinpei nakata */

import("konoha.bytes");

void test () {
	 assert("hoge".toBytes().encodeTo("EUC-JP").decodeFrom("EUC-JP") == "hoge");
	 assert("ほげほげ".toBytes().encodeTo("EUC-JP").decodeFrom("EUC-JP") == "ほげほげ");
}

test();
