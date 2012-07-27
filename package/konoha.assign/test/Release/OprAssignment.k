K.import("konoha.assign");

void test_oprAssign() {
	int i = 10;
	i += 1;
	p(i);
	i -= 3;
	p(i);
	i *= 2;
	p(i);
	i /= 2;
	p(i);
	i %= 3;
	p(i);
}

test_oprAssign();
