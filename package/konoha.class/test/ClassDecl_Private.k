// written by kimio kuramitsu

K.import("konoha.class");

@Private class Point {  // @Test 
	int x;
	int y;
}

void test() {
	Point p = new Point();
	assert(p.square() == 0);
}
