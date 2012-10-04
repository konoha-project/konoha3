// written by wakamatsu
// modified by chenji

import("konoha.io");
import("konoha.new");

void test() {
	File file1 = new File("../../../package/konoha.io/test/test.txt");
	assert(file1.exists() == true);
	File file2 = new File("../../../package/konoha.io/test/hoge.txt");
	assert(file2.exists() == false);
}

test();

