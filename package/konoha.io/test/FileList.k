// written by wakamatsu
import("konoha");
import("konoha.io");

const b = "../../../package/konoha.io/test/";

void test() {
	File file = new File(b + "testdir");
	Array[String] l = file.list();
	System.p(l.size);
	int i = 0;
	while(i < l.size) {
		System.p(l[i]);
		i += 1;
	}
}

void testNoSuchDirectoryCase() {
	File file = new File(b + "hoge");
	Array[String] l = file.list();
	System.p(l.size);
}

test();
testNoSuchDirectoryCase();

