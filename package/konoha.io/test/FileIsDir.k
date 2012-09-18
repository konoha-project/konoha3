// written by wakamatsu
K.import("konoha");
K.import("konoha.io");

const b = "../../../package/konoha.io/test/";

void test() {
	File file1 = new File(b + "test.txt");
	p(file1.isDir());
	File file2 = new File(b + "testdir");
	p(file2.isDir());
	p(System.isDir(b + "test.txt"));
	p(System.isDir(b + "testdir"));
}

test();

