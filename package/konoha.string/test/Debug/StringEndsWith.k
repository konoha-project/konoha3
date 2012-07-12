//written by takeo onoda

K.import("konoha.string");

void test() {
	System.p("foobar".endsWith("bar"));
	System.p("foobar".endsWith("foobar"));
	System.p("foobar".endsWith("0foobar"));
	System.p("aあiいuう".endsWith("iいuう"));
	System.p("aあiいuう".endsWith("aあiいuう"));
	System.p("aあiいuう".endsWith("0aあiいuう"));
	System.p("".endsWith(""));
}

test();
