//written by takeo onoda

K.import("konoha.string");

void test() {
	System.p("foobar".lastIndexOf("f"));
	System.p("foobar".lastIndexOf("o"));
	System.p("foobar".lastIndexOf("A"));
	System.p("foobarfoobar".lastIndexOf("bar"));
	System.p("foobarfoobar".lastIndexOf("barr"));
	System.p("aあiいuう".indexOf("a"));
	System.p("aあiいuう".indexOf("あ"));
	System.p("aあiいuう".indexOf("I"));
	System.p("aあiいuう".indexOf("いuう"));
	System.p("aあiいuう".indexOf("いuうe"));
}

test();
