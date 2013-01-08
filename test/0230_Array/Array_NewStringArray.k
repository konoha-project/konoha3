// written by kimio kuramitsu
import("Syntax.JavaNewArray");
import("JavaScript.Array");

void test() {
	String[] a = new String[10];
	assert(a.getSize() == 10);
}

test();

