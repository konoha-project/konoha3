// written by chenji

import("konoha.foreach");
import("JavaScript.Array");

void test() {
	String[] slist = ["a", "b", "c", "d"];
	int i = 0;
	for(String s in slist) {
		assert(s == slist[i]);
		i = i + 1;
	}
	i = 0;
	for(s in slist) {
		assert(s == slist[i]);
		i = i + 1;
	}
}

test();
