// written by wakamatsu
K.import("konoha");
K.import("konoha.io");

String b = "../../../package/konoha.io/test/Debug/";

void test() {
	File file = new File(b + "testdir");
	Array[String] l = file.list();
	p(l.size);
	int i = 0;
	while(i < l.size) {
		p(l[i]);
		i += 1;
	}
}

test();

