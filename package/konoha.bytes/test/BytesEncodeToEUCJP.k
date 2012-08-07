/* written by shinpei nakata */

K.import("konoha.bytes");

void test () {
	assert("hoge".asBytes().encodeTo("EUC-JP").asString() == "hoge");
	p("ほげほげ".asBytes().encodeTo("EUC-JP").asString());
}

test();
