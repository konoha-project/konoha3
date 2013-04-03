import("Konoha.ClosureModel");

Func[int, int] f() {
	return function (int x, int y) int {
		return x + y;
	}
}

void test() {
	assert(f()(1, 2) == 3);
}

test();
