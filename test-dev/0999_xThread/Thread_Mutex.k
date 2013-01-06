// written by wakamatsu
//import("konoha.new"); Don't use this
import("posix.thread");

void test() {
	Mutex m = new Mutex();

	m.lock();
	assert(m.trylock() == false); // fail
	m.unlock();

	assert(m.trylock() == true); // success
	m.unlock();
}

test();

