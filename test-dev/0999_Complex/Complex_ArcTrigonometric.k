import("Type.Complex");
import("JavaScript.Math");
import("Type.Float");
//import("konoha.new"); Don't use this
import("Syntax.JavaStyleClass");



void test() {
	Complex c = new Complex(1.57, 2.71);
	assert(c.casin()   - 0.503815016814584 <= FLOAT_EPSILON*10);
	assert(c.cacos()   - 1.06698130998031  <= FLOAT_EPSILON*10);
	assert(c.catan()   - 1.39959005476118  <= FLOAT_EPSILON*10);
}

test();
