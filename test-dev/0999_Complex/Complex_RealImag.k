import("Type.Complex");
import("JavaScript.Math");
import("Type.Float");
//import("konoha.new"); Don't use this
import("Syntax.JavaStyleClass");



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
