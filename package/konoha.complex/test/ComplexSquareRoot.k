import("konoha.complex");
import("konoha.math");
import("konoha.float");
import("konoha.new");
import("konoha.class");



void test() {
	Complex c = new Complex(1.57, 2.71);
	System.p(c.csqrt());
	System.p(c.csqrtf());
	System.p(c.csqrtl());
}

test();
