//written by takeo onoda

K.import("konoha.string");

void test() {
	System.p("foobar".startsWith("foo"));
	System.p("foobar".startsWith("foobar"));
	System.p("foobar".startsWith("foobarhoge"));
	System.p("aあiいuう".startsWith("aあiい"));
	System.p("aあiいuう".startsWith("aあiいuう"));
	System.p("aあiいuう".startsWith("aあiいuうe"));
	System.p("".startsWith(""));
}

test();
