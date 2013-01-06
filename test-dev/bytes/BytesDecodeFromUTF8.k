/* written by shinpei nakata */

import("Type.Bytes");

void test () {
	 assert("ほげほげ".toBytes().encodeTo("UTF-8").decodeFrom("UTF-8") == "ほげほげ");
}

test();

