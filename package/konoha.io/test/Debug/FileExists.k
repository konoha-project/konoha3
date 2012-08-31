// written by wakamatsu
K.import("konoha");
K.import("konoha.io");

String b = "../../../package/konoha.io/test/Debug/";

void test() {
	File file1 = new File(b + "test.txt");
	p(file1.exists());
	File file2 = new File(b + "hoge.txt");
	p(file2.exists());
}

test();

