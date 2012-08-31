//written by takeo onoda

K.import("konoha.string");

void test() {
	assert("hoge".getSize() == 4);
	assert("aあiいuう".getSize() == 6);
	assert("".getSize() == 0);
}

test();
