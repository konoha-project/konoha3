import("konoha.complex");
import("konoha.math");
import("konoha.float");
import("konoha.new");
import("konoha.class");



void test() {
	Complex c = new Complex(1.57, 2.71);
	System.p(c.csin());
	System.p(c.csinf());
	System.p(c.csinl());
	System.p(c.ccos());
	System.p(c.ccosf());
	System.p(c.ccosl());
	System.p(c.ctan());
	System.p(c.ctanf());
	System.p(c.ctanl());
}

test();
