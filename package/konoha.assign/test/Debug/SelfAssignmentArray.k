// written by kimio kuramitsu

K.import("konoha.assign");
K.import("konoha.array"); // if Ok

void test() {
	int[] a = new int[2];
	assert(a[0] == 0);
	a[0] += 1;   // watch ##
	assert(a[0] == 1);
}

test();



