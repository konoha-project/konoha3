// written by imasahiro

import("konoha.array");

int f(int x) { return x + 1; }
int g(int x) { return x - 1; }

void test () {
	int[] a = [10, 20, 30, 40];

	int[] ret = a.map(f);
	assert(a.getSize() == 4);
	assert(a[0] == 10);
	assert(a[1] == 20);
	assert(a[2] == 30);
	assert(a[3] == 40);

	assert(ret.getSize() == 4);
	assert(ret[0] == 11);
	assert(ret[1] == 21);
	assert(ret[2] == 31);
	assert(ret[3] == 41);
}

test();
