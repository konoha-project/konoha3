// written kimio kuramitsu

import("Syntax.JavaStyleClass");

@Singleton class Math {  // @Test
	int abs(int x) {     // This must be a static method
		if(x < 0) return -x;
		return x;
	}
}

void test() {
	assert(Math.abs(-10) == 10);
}

test();

