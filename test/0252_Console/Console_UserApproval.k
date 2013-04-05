// written by kimio

import("Konoha.Console");

void test() {
	boolean isOkay = Console.inputUserApproval("Is it okay?", "OK", "NOP", true);  // OutputTest returns true;
	assert(isOkay);
}

test();

