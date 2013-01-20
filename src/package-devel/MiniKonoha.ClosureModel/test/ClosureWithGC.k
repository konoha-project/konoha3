import("MiniKonoha.ClosureModel");

Func[String, String] f() {
	String s = "hi";
	return function (String t) String {
		s = s + t;
		return s;
	}
}

void test() {
	Func[String, String] g = f();
	System.gc();
	assert(g(", ") == "hi, ");
	System.gc();
	assert(g("bye") == "hi, bye");
}
