// written by kimio kuramitsu

K.import("konoha.class");

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
