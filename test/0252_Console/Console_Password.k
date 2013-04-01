// written by kimio

import("Konoha.Console");

void test() {
	String line = Console.inputUserPassword("Password: ");  // OutputTest returns null;
	assert(line == "");
}

test();

