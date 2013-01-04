// written by kimio kuramitsu

import("Syntax.JavaStyleClass");

@Private class Point {  // @Test 
	int x;
	int y;
}

void test() {
	Point p = new Point();
	assert(p.square() == 0);
}
