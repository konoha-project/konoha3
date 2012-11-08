// written by kimio kuramitsu

import("konoha.global");
AllowImplicitGlobalVariable(true);

int x = 123;

void test() {
	assert(x == 123);
}

test();

