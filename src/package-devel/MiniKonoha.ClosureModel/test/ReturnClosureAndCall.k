import("MiniKonoha.ClosureModel");

Func[int, int] f() {
	return function (int x, int y) int {
		return x + y;
	}
}

void test() {
	assert(test()(1, 2) == 3);
}
