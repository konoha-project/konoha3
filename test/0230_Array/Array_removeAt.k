import("JavaScript.Array");

void test() {
	int[] a = [1];
	a.removeAt(0);
	a.add(1);
	assert(a.getSize() == 1);
}

test();
