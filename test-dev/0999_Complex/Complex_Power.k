import("Type.Complex");
import("JavaScript.Math");
import("Type.Float");
//import("konoha.new"); Don't use this
import("Syntax.JavaStyleClass");



void test() {
	Complex c = new Complex(1.57, 2.71);
	System.p(c.cpow(90, 180));
	System.p(c.cpowf(90, 180));
	System.p(c.cpowl(90, 180));
}

test();
