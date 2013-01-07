/*
http://projecteuler.net/problem=16


215 = 32768 and the sum of its digits is 3 + 2 + 7 + 6 + 8 = 26.
What is the sum of the digits of the number 21000?

*/
import("JavaScript.String");
import("Syntax.CStyleWhile");
import("Syntax.CStyleCompoundAssignment");
import("Syntax.JavaStyleClass");
import("Lib.Gmp");

Mpz factorial(Mpz m, int n) {
	int i = 1;
	Mpz ret = m;
	while(i < n) {
		ret *= m;
		i += 1;
	}
	return ret;
}

void main() {
	Mpz m = new Mpz("2");
	m = factorial(m,1000);
	String str = m.toString();
	int i = 0;
	int ans = 0;
	while(i < str.length) {
		ans += str.charCodeAt(i) - 48;
		i   += 1;
	}
	assert(ans == 1366); //=> 1366
}

main();
