import("konoha.closure");

void test() {
	Func[int, int] f = function (int x) => int{
		return x + 1;
	}
	assert(f(1) == 2);
}

test();
