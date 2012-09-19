// written by kimio kuramitsu

import("konoha.class");
import("konoha.assign");

@Interface class Point {
	int x;
	int y;
}

int Point.square() {  // @Test this must be forbidden
	return x * y;
}

void test() {
	Point p = new Point();
	assert(p.square() == 0); // This must be undefined method
}
