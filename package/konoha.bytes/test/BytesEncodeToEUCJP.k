/* written by shinpei nakata */

K.import("konoha.bytes");

void test () {
	 assert("hoge".asBytes().encodeTo("EUC-JP").asString() == "hoge");
	 assert("ほげほげ".asBytes().encodeTo("EUC-JP").asString() == "ほげほげ");
}

test();
