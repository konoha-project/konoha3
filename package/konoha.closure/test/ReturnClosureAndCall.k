import("konoha.closure");

Func[int, int] f() {
	return function (int x) => int{
		return x + 1;
	}
}

void test() {
	assert(test()(1) == 2);
}
