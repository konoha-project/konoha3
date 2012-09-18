/* written by shinpei nakata */

K.import("konoha.bytes");

void test () {
	assert("hoge".asBytes().encodeTo("SHIFT_JIS").asString() == "hoge");
	p("ほげほげ".asBytes().encodeTo("SHIFT_JIS").asString());
}

test();
