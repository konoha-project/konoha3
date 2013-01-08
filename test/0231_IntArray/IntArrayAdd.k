// written by kimio kuramitsu
import("Syntax.JavaNewArray");
import("JavaScript.Array");

void test() {
	int[] a = new int[0];
	a.add(1);
	a.add(2);
	a.add(9);
	assert(a.getSize() == 3);
}

test();

