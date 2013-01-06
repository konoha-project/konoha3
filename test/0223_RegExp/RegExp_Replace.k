import("JavaScript.Regexp");

void test() {
	assert("hoge1234".replace(/123/, "xxx") == "hogexxx4");
	assert("hoge1234".replace(/\d/, "xxx") == "hogexxx234");
	assert("hoge1234".replace(/x/, "xxx") == "hoge1234");
	assert("hoge/1234".replace(/\//, "xxx") == "hogexxx1234");
	assert("hoge1234".replace(/1.*/, "xxx") == "hogexxx");
}

test();
