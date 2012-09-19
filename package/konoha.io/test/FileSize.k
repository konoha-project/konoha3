// written by wakamatsu
import("konoha");
import("konoha.io");

const b = "../../../package/konoha.io/test/";

void test() {
	File file = new File(b + "test.txt");
	System.p(file.getSize());
}

test();

