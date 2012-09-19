// written by wakamatsu
import("posix.thread");

void f() {
	System.p("hello world");
}

void test() {
	Thread t1 = Thread.create(f);
	t1.join();
}

test();

