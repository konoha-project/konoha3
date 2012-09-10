// written by wakamatsu
K.import("konoha");
K.import("konoha.io");

const b = "../../../package/konoha.io/test/Debug/";

void test() {
	File file = new File(b + "test.txt");
	p(file.getSize());
}

test();

