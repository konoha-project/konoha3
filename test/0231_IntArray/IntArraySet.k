// written by kimio kuramitsu
import("Syntax.JavaNewArray");
import("JavaScript.Array");

void test() {
	int[] a = new int[0];
	a.add(1);
	a.add(2);
	a.add(9);
	assert(a.getSize() == 3);
	assert(a.get(2) == 9);
	a.set(2, 3);
	assert(a.getSize() == 3);
	assert(a.get(2) == 3);
}


test();

