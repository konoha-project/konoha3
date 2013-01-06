/*
http://projecteuler.net/problem=15

Starting in the top left corner of a 2^2 grid, there are 6 routes (without backtracking) to the bottom right corner.


How many routes are there through a 20^20 grid?

*/

import("Syntax.CStyleWhile");
import("Syntax.CStyleCompoundAssignment");
import("Syntax.JavaStyleClass");
import("Lib.Gmp")

Mpz fact(int n){
	Mpz f = new Mpz(1);
	while(n > 1){
		f *= n;
		n -= 1;
	}
	return f;
}

Mpz permut(int n, int r){
	Mpz f = new Mpz(1);
	int limit = n - r;
	while(n > limit){
		f *= n;
		n -= 1;
	}
	return f;
}

Mpz routes(int w, int h){
	return permut(w+h, w) / fact(h)
}

void main() {
	assert(routes(2, 2) == 6);
	assert(routes(20, 20) == "137846528820".toMpz());
}

main();
