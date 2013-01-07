// written by imasahiro
// modified by chenji

import("konoha.foreach");
import("JavaScript.Array");

void test() {
	int[] ilist = [1, 2, 3, 4];
	int i = 0;
	for(int a in ilist) {
		assert(a == ilist[i]);
		i = i + 1;
	}
	i = 0;
	for(a in ilist) {
		assert(a == ilist[i]);
		i = i + 1;
	}
}

test();
