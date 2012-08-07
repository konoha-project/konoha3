/* written by shinpei nakata */

K.import("konoha.bytes");

void test () {
	 assert("hoge".asBytes().encodeTo("UTF-8").asString() == "hoge");
	 assert("ほげほげ".asBytes().encodeTo("UTF-8").asString() == "ほげほげ");
}

test();
