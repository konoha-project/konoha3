// written by wakamatsu
K.import("konoha.float");
K.import("konoha.bytes");
K.import("konoha.mpi");

void test() {
	MPIComm w = MPIComm.getWorld();
	p("hello world. rank = " + w.getRank() + " / " + w.getSize());
}

test();

