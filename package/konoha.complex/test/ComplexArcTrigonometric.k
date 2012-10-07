import("konoha.complex");
import("konoha.math");
import("konoha.float");
import("konoha.new");
import("konoha.class");



void test() {
	Complex c = new Complex(1.57, 2.71);
	System.p(c.casin());
	System.p(c.casinf());
	System.p(c.casinl());
	System.p(c.cacos());
	System.p(c.cacosf());
	System.p(c.cacosl());
	System.p(c.catan());
	System.p(c.catanf());
	System.p(c.catanl());
}

test();
