// written by wakamatsu
K.import("konoha.new");
K.import("posix.thread");

void test() {
	Mutex m = new Mutex();

	m.lock();
	assert(m.tryLock() == false); // fail
	m.unlock();

	assert(m.tryLock() == true); // success
	m.unlock();
}

test();

