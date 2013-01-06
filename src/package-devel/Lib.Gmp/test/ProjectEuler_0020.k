/*
  http://projecteuler.net/problem=20

  n! means n  (n  1)  ...  3  2  1
  For example, 10! = 10  9  ...  3  2  1 = 3628800,and the sum of the digits in the number 10! is 3 + 6 + 2 + 8 + 8 + 0 + 0 = 27.
  Find the sum of the digits in the number 100!

*/
import("Syntax.JavaStyleClass");
import("JavaScript.Array");
import("Syntax.CStyleWhile");
import("Syntax.CStyleCompoundAssignment");
import("Lib.Gmp")

Mpz fact(int n){
	Mpz f = new Mpz(1);
	while(n > 1){
		f *= n;
		n -= 1;
	}
	return f;
}

void main() {
	Mpz f100 = fact(100);
	int sum = 0;
	while(f100 > 0){
		sum += f100 % 10;
		f100 /= 10;
	}
	assert(sum == 648);
}

main();
