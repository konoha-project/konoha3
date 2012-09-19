//written by takeo onoda

import("konoha.string");

void test() {
	assert("hogefuga".opHAS("hoge") == true);
	assert("hogefuga".opHAS("fuga") == true);
	assert("hogefuga".opHAS("hogefuga") == true);
	assert("hogefuga".opHAS("0hogefuga") == false);
	assert("hogefuga".opHAS("hogefuga0") == false);
	assert("aあiい".opHAS("aあ") == true);
	assert("aあiい".opHAS("iい") == true);
	assert("aあiい".opHAS("aあiい") == true);
	assert("aあiい".opHAS("0aあiい") == false);
	assert("aあiい".opHAS("aあiい0") == false);
	assert("".opHAS("") == true);
}

test();
