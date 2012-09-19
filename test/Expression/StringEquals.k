//written by takeo onoda

import("konoha.string");

void test() {
	System.p("hoge" == "h");
	System.p("hoge" == "hoge");
	System.p("hoge" == "hogeee");
	System.p("aあiいuう" == "a");
	System.p("aあiいuう" == "aあiいuう");
	System.p("aあiいuう" == "aあiいuうe");
	System.p("" == "");
}

test();
