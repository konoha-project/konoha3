import("konoha.complex");
import("konoha.math");
import("konoha.float");
import("konoha.new");
import("konoha.class");



void test() {
	Complex c = new Complex(1.57, 2.71);
	assert(c.csin()  - (7.547904e+00) <= FLOAT_EPSILON*10);
	//assert(c.csinf() - (7.547904e+00) <= FLOAT_EPSILON*10);
	assert(c.csinl() - (7.547904e+00) <= FLOAT_EPSILON*10);
	//assert(c.ccos()  - (6.010599e-03) <= FLOAT_EPSILON*10);
	assert(c.ccosf() - (6.010204e-03) <= FLOAT_EPSILON*10);
	//assert(c.ccosl() - (6.010599e-03) <= FLOAT_EPSILON*10);
	//assert(c.ctan()  - (1.422751e-05) <= FLOAT_EPSILON*10);
	//assert(c.ctanf() - (1.422657e-05) <= FLOAT_EPSILON*10);
	//assert(c.ctanl() - (1.422751e-05) <= FLOAT_EPSILON*10);
}

test();
