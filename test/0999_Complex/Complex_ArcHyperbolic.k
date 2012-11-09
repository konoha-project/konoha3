import("konoha.complex");
import("konoha.math");
import("konoha.float");
import("konoha.new");
import("konoha.class");



void test() {
	Complex c = new Complex(1.57, 2.71);
	System.p(c.casinh());
	System.p(c.casinhf());
	System.p(c.casinhl());
	System.p(c.cacosh());
	System.p(c.cacoshf());
	System.p(c.cacoshl());
	System.p(c.catanh());
	System.p(c.catanhf());
	System.p(c.catanhl());
}

test();
