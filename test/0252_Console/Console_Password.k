// written by kimio

import("konoha.console");

void test() {
	String line = Console.inputUserPassword("Password: ");  // OutputTest returns null;
	assert(line == "");
}

test();

