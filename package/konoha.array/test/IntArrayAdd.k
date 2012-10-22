// written by kimio kuramitsu
import("konoha.new");
import("konoha.array");

void test() {
	int[] a = new int[0];
	a.add(1);
	a.add(2);
	a.add(9);
	assert(a.getSize() == 3);
}

test();

