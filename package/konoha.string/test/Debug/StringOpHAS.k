//written by takeo onoda

K.import("konoha.string");

void test() {
	System.p("hogefuga".opHAS("hoge"));
	System.p("hogefuga".opHAS("fuga"));
	System.p("hogefuga".opHAS("hogefuga"));
	System.p("hogefuga".opHAS("0hogefuga"));
	System.p("hogefuga".opHAS("hogefuga0"));
	System.p("aあiい".opHAS("aあ"));
	System.p("aあiい".opHAS("iい"));
	System.p("aあiい".opHAS("aあiい"));
	System.p("aあiい".opHAS("0aあiい"));
	System.p("aあiい".opHAS("aあiい0"));
	System.p("".opHAS(""));
}

test();
