// written by shinpei

import("JavaScript.Array");

void f() {
	Array[String] sa = ["a","b","c"];
	assert(sa[0] == "a");
	assert(sa[1] == "b");
	assert(sa[2] == "c");
}
f();
