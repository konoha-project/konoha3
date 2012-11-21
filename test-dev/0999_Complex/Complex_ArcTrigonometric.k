import("konoha.complex");
import("konoha.math");
import("konoha.float");
import("konoha.new");
import("konoha.class");



void test() {
	Complex c = new Complex(1.57, 2.71);
	assert(c.casin()   - 0.503815016814584 <= FLOAT_EPSILON*10);
	assert(c.cacos()   - 1.06698130998031  <= FLOAT_EPSILON*10);
	assert(c.catan()   - 1.39959005476118  <= FLOAT_EPSILON*10);
}

test();
