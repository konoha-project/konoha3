import("Lib.Gmp");
import("Syntax.JavaStyleClass")
import("cstyle")

void main() {
	Mpz n1 = new Mpz(10000);
	Mpz n2 = new Mpz("100001000000000");
	Mpz n3 = new Mpz(n1);
	Mpz n4 = "100001000010000".toMpz();

	assert(n1 == n3);
	assert(n1 + n2 == n4);
	assert(n1 == 10000);
	assert(n4 - n2 == n1);
	assert(n2 - n4 == -n1);

	assert(n1 * 2 == 2 * n1);
	assert(n1 * 2 > n1);
	assert(n1 * 2 >= n1);
	assert(n1 / 2 < n1);
	assert(n1 / 2 <= n1);
	assert(3 * n1 / 2 != n1);

	assert((11111).toMpz() % 10000 == 1111);

	assert((10).toMpz().power(100).size == 101);
	assert(n4.isEven());

	Mpz m6 = (6).toMpz();
	Mpz m2 = (2).toMpz();
	Mpz m3 = (3).toMpz();

	assert(m6 / 2  == 3);
	assert(m6 / m2 == 3);
	assert(m6 / m2 == m3);
	assert(6  / m2 == m3);
	assert(6  / 2  == m3);
	assert(6  / m2 == 3);
}

main();
