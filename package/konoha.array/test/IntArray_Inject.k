import("konoha.array");

int add(int a, int b) {
	return a+b;
}

void test() {
	int[] a = [1,2,3];
	assert(a.inject(add) == 6);
}

test();
