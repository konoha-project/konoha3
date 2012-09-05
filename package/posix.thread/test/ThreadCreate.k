// written by wakamatsu
K.import("posix.thread");

void f() {
	p("hello world");
}

void test() {
	Thread t1 = Thread.create(f);
	t1.join();
}

test();

