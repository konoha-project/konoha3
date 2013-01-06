/*
http://projecteuler.net/problem=3

The prime factors of 13195 are 5, 7, 13 and 29.
What is the largest prime factor of the number 600851475143 ?


*/

import("Syntax.CStyleWhile");
import("Syntax.CStyleCompoundAssignment");
import("JavaScript.Math");
import("Lib.Gmp");

int divid() {
	int i = 2, max = 0, tmp, counter = 0;
	Mpz number = "600851475143".toMpz();

	int r = Math.sqrt(number.toint());

	while(i != r) {
		if((tmp = number % i) == 0) {
			max = i;
			while((number % i) == 0) {
				number = number / i;
			}
		}
		i += 1;
	}
	return max;
}

assert(divid() == 6857); //-> 6857

