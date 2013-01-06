// written by chenji

import("konoha.foreach");
import("JavaScript.Array");
import("Type.Float");

void test() {
	float[] flist = [1.0, 2.0, 3.0, 4.0];
	int i = 0;
	for(float f in flist) {
		assert(f == flist[i]);
		i = i + 1;
	}
	i = 0;
	for(f in flist) {
		assert(f == flist[i]);
		i = i + 1;
	}
}

test();
