import("konoha.complex");
import("konoha.math");
import("konoha.float");
import("konoha.new");
import("konoha.class");



void test() {
	Complex c = new Complex(1.57, 2.71);
	System.p(c.csinh());
	System.p(c.csinhf());
	System.p(c.csinhl());
	System.p(c.ccosh());
	System.p(c.ccoshf());
	System.p(c.ccoshl());
	System.p(c.ctanh());
	System.p(c.ctanhf());
	System.p(c.ctanhl());
}

test();
