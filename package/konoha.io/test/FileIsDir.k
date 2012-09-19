// written by wakamatsu
import("konoha");
import("konoha.io");

const b = "../../../package/konoha.io/test/";

void test() {
	File file1 = new File(b + "test.txt");
	System.p(file1.isDir());
	File file2 = new File(b + "testdir");
	System.p(file2.isDir());
	System.p(System.isDir(b + "test.txt"));
	System.p(System.isDir(b + "testdir"));
}

test();

