// written by wakamatsu
import("konoha");
import("konoha.io");
import("konoha.const");

const b = "../../../package/konoha.io/test/";

void test() {
	File file1 = new File(b + "test.txt");
	System.p(file1.exists());
	File file2 = new File(b + "hoge.txt");
	System.p(file2.exists());
}

test();

