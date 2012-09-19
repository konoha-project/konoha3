//written by takeo onoda

import("konoha.string");

void test() {
	System.p("hoge" != ("hog"));
	System.p("hoge" != ("oge"));
	System.p("hoge" != ("hoge"));
	System.p("hoge" != ("0hoge"));
	System.p("hoge" != ("hoge0"));
	System.p("aあiいuう" != ("aあiいu"));
	System.p("aあiいuう" != ("あiいuう"));
	System.p("aあiいuう" != ("aあiいuう"));
	System.p("aあiいuう" != ("0aあiいuう"));
	System.p("aあiいuう" != ("aあiいuう0"));
	System.p("" != (""));
}

test();
