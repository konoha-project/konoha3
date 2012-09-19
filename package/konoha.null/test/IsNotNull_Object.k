// written by shinpei nakata

import("konoha.null");
import("konoha.object");

void test () {
	Object obj = null;
	assert(obj.isNotNull() == false);
}

test();
