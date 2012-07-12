//written by takeo onoda

K.import("konoha.string");

void test() {
	System.p("foobar".substring(0, 3));
	System.p("foobar".substring(3, 3));
	System.p("foobar".substring(3, 6));
	System.p("foobar".substring(1, 0));
	//	System.p("foobar".substring(10, 3));
	//	System.p("foobar".substring(-2, 4));
	System.p("foobar".substring(2, -2));
	System.p("aあiいuう".substring(0, 3));
	System.p("aあiいuう".substring(3, 3));
	System.p("aあiいuう".substring(3, 6));
	System.p("aあiいuう".substring(1, 0));
	//	System.p("aあiいuう".substring(10, 3));
	//	System.p("aあiいuう".substring(-2, 4));
	System.p("aあiいuう".substring(2, -2));
}

test();
