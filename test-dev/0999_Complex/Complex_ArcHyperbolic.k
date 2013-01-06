import("Type.Complex");
import("JavaScript.Math");
import("Type.Float");
//import("konoha.new"); Don't use this
import("Syntax.JavaStyleClass");



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
