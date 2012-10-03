import("konoha.complex");
import("konoha.math");
import("konoha.float");
import("konoha.new");
import("konoha.class");



void test() {
	Complex c = new Complex(1.57, 2.71);
	System.p(c.creal());
	System.p(c.crealf());
	System.p(c.creall());
	System.p(c.cimag());
	System.p(c.cimagf());
	System.p(c.cimagl());
}

test();
