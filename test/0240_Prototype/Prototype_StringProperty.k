// written by kimio kuramitsu

import("JavaScript.Prototype");

void test() {
	Prototype p = new Prototype();
	p.set("name", "naruto");
	String s = p.get("name");
	assert(s == "naruto");
}

test();

