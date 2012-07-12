//written by takeo onoda

K.import("konoha.string");

void test() {
	System.p("foobar".indexOf("f"));
	System.p("foobar".indexOf("o"));
	System.p("foobar".indexOf("A"));
	System.p("foobar".indexOf("bar"));
	System.p("foobar".indexOf("barr"));
	System.p("aあiいuう".indexOf("a"));
	System.p("aあiいuう".indexOf("あ"));
	System.p("aあiいuう".indexOf("I"));
	System.p("aあiいuう".indexOf("いuう"));
	System.p("aあiいuう".indexOf("いuうe"));
}

test();
