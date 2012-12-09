// written by kimio kuramitsu

import("konoha.prototype");

void test() {
	Prototype p = new Prototype();
	p.set("age", 17);
	int age = p.get("age");
	assert(age == 17);
}

test();

