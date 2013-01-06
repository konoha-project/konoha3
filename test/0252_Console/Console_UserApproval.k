// written by kimio

import("MiniKonoha.Console");

void test() {
	boolean isOkay = Console.inputUserApproval("Is it okay?", "OK", "NOP", true);  // OutputTest returns true;
	assert(isOkay);
}

test();

