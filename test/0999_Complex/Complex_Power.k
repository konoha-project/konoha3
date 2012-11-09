import("konoha.complex");
import("konoha.math");
import("konoha.float");
import("konoha.new");
import("konoha.class");



void test() {
	Complex c = new Complex(1.57, 2.71);
	System.p(c.cpow(90, 180));
	System.p(c.cpowf(90, 180));
	System.p(c.cpowl(90, 180));
}

test();
