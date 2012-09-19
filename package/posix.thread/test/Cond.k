// written by wakamatsu
import("konoha");
import("konoha.global");
import("posix.thread");
setTransparentGlobalVariable(true);

Mutex m = new Mutex();
Cond c = new Cond();
int counter = 0;

void test_1() {
	m.lock();
	while(counter == 0) {
		c.wait(m);
	}
	m.unlock();
	assert(counter != 0);
}

void test() {
	Thread t = Thread.create(test_1);

	m.lock();
	counter += 1;
	c.signal();
	m.unlock();

	t.join();
}

test();

