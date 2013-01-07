// written by kimio kuramitsu

import("Syntax.GlobalVariable");
AllowImplicitGlobalVariable(true);

int x = 123;

void test() {
	assert(x == 123);
}

test();

