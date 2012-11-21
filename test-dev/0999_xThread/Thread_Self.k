// written by wakamatsu
import("posix.thread");

void f() { }

void test() {
	Thread t1 = Thread.self();
	Thread t2 = Thread.self();
	assert(t1.equal(t2) == true);

	Thread t3 = Thread.create(f);
	assert(t1.equal(t3) == false);
	assert(t2.equal(t3) == false);

	t3.join();
}

test();

