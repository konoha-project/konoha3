import("JavaScript.Array");
import("JavaScript.Regexp");

void test() {
	Array[String] r1 = "aa/bb/cc/dd/ee".split(/\//);
	assert(r1[0] == "aa");
	assert(r1[1] == "bb");
	assert(r1[2] == "cc");
	assert(r1[3] == "dd");
	assert(r1[4] == "ee");
	Array[String] r2 = "hoge1234hoge1234".split(/[gh]/);
	assert(r2[0] == "");
	assert(r2[1] == "o");
	assert(r2[2] == "e1234");
	assert(r2[3] == "o");
	assert(r2[4] == "e1234");
}

test();
