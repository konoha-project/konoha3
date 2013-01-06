// written by kimio

import("MiniKonoha.Console");

void test() {
	String line = Console.inputUserPassword("Password: ");  // OutputTest returns null;
	assert(line == "");
}

test();

