import("MiniKonoha.ClosureModel");

Func[int, int] f() {
	int y = 0;
	return function (int x) int {
		y = y + x;
		return y;
	}
}

void test() {
	Func[int, int] g = f();
	assert(g(1) == 1);
	assert(g(2) == 3);
}
