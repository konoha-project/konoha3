/* written by shinpei nakata */

K.import("konoha.bytes");

void test () {
	 assert("ほげほげ".asBytes().encodeTo("UTF-8").decodeFrom("UTF-8") == "ほげほげ");
}

test();

