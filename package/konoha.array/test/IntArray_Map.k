import("konoha.array");

int func(int num) {
	return num*2;
}

void test() {
	int[] a = [1,2,3];
	int[] b = a.map(func);
	assert(a[0] == 1);
	assert(a[1] == 2);
	assert(a[2] == 3);

	assert(b[0] == 2);
	assert(b[1] == 4);
	assert(b[2] == 6);
}

test();
