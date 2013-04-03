import("Konoha.ClosureModel");

Func[int, int] f() {
	return function (int x) int {
		return x + 1;
	}
}

Func[int, int] g(int a) {
	int b = a + 100;
	return function (int x) int {
		return x + b + 1;
	}
}

void test() {
	Func[int, int] x = f();
	assert(x(10) == 11);

	Func[int, int] y = g(10);
	assert(y(20) == 131);
}

test();
