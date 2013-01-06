/*
http://projecteuler.net/problem=25

The Fibonacci sequence is defined by the recurrence relation:
Fn = Fn1 + Fn2, where F1 = 1 and F2 = 1.
Hence the first 12 terms will be:
F1 = 1
F2 = 1
F3 = 2
F4 = 3
F5 = 5
F6 = 8
F7 = 13
F8 = 21
F9 = 34
F10 = 55
F11 = 89
F12 = 144
The 12th term, F12, is the first term to contain three digits.
What is the first term in the Fibonacci sequence to contain 1000 digits?

*/
import("Syntax.JavaStyleClass");
import("Syntax.CStyleWhile");
import("Syntax.Null");
import("Syntax.CStyleCompoundAssignment");
import("Lib.Gmp")

void main() {
	Mpz fa  = null;
	Mpz fb  = new Mpz("1");
	Mpz fc  = new Mpz("2");
	Mpz ret = null;
	int limit = 1000;
	int n = 3;

	while(true){
		fa = fb + fc; n += 1;
		if(fa.size == limit) {
			ret = fa;
			break;
		}
		fb = fc + fa; n += 1;
		if(fb.size == limit) {
			ret = fb;
			break;
		}
		fc = fa + fb; n += 1;
		if(fc.size == limit) {
			ret = fc;
			break;
		}
	}
	assert(n == 4782);
}

main();
