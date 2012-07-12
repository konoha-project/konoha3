//written by takeo onoda

K.import("konoha.string");

void test() {
	System.p("hoge".opNEQ("hog"));
	System.p("hoge".opNEQ("oge"));
	System.p("hoge".opNEQ("hoge"));
	System.p("hoge".opNEQ("0hoge"));
	System.p("hoge".opNEQ("hoge0"));
	System.p("aあiいuう".opNEQ("aあiいu"));
	System.p("aあiいuう".opNEQ("あiいuう"));
	System.p("aあiいuう".opNEQ("aあiいuう"));
	System.p("aあiいuう".opNEQ("0aあiいuう"));
	System.p("aあiいuう".opNEQ("aあiいuう0"));
	System.p("".opNEQ(""));
}

test();
