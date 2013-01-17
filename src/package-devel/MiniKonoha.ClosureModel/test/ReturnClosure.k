import("MiniKonoha.ClosureModel");

Func[int, int] f() {
	return function (int x) int {
		return x + 1;
	}
}

void test() {
	f();
}

test();
