// written by wakamatsu
K.import("konoha");
K.import("konoha.global");
K.import("posix.thread");
K.setTransparentGlobalVariable(true);

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

