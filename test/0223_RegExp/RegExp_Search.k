import("JavaScript.Regexp");

void test() {
	assert("hoge1234".search(/123/) == 4);
	assert("hoge1234".search(/\d/) == 4);
	assert("hoge1234".search(/x/) == -1);
	assert("hoge/1234".search(/\//) == 4);
	assert("hoge1234".search(/1.*/) == 4);
}

test();
