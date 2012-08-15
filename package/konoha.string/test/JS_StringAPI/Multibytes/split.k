//written by Joseph

K.import("konoha.string");

void test() {
	assert("How are you doing today?".split(" ") == "How,are,you,doing,today?");
	assert("How are you doing today?".split() == "How are you doing today?");
	assert("How are you doing today?".split("o") == "H,w are y,u d,ing t,day?");
}

test();
