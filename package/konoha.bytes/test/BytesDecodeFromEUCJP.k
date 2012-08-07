/* written by shinpei nakata */

K.import("konoha.bytes");


void test () {
	 //p("'" + "ほげほげ".asBytes().encodeTo("EUC-JP").decodeFrom("EUC-JP") + "'");
	 assert("ほげほげ".asBytes().encodeTo("EUC-JP").decodeFrom("EUC-JP") == "ほげほげ");
}

test();
