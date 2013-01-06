// written by kimio

import("Syntax.JavaStyleClass");

class Counter {
	int count;
	Counter(int count) {
		this.count = count;
	}
	void count() {
		this.count = this.count + 1;
	}
	void reset() {
		this.count = 0;
	}
}

void test() {
	Counter c = new Counter(0);
	c.count();
	assert(c.count == 1);
	c.count();
	assert(c.count == 2);
	c.reset();
	assert(c.count == 0);
}

test();

