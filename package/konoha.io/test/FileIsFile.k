// written by wakamatsu
import("konoha");
import("konoha.io");

const b = "../../../package/konoha.io/test/";

void test() {
	File file1 = new File(b + "test.txt");
	System.p(file1.isFile());
	File file2 = new File(b + "testdir");
	System.p(file2.isFile());
	System.p(System.isFile(b + "test.txt"));
	System.p(System.isFile(b + "testdir"));
}

test();

