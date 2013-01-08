// written by kimio kuramitsu
import("Syntax.JavaNewArray");
import("JavaScript.Array");
import("JavaStyle.Object");

void test() {
	Object[] a = new Object[10];
	assert(a.getSize() == 10);
}

test();

