// written by wakamatsu
K.import("konoha");
K.import("konoha.io");

String b = "../../../package/konoha.io/test/Debug/";

void test() {
	File file1 = new File(b + "test.txt");
	p(file1.isDir());
	File file2 = new File(b + "testdir");
	p(file2.isDir());
}

test();

