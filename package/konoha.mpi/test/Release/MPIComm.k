// written by wakamatsu
import("konoha.mpi");

void test() {
	MPIComm w = MPIComm.getWorld();
	System.p("hello world. rank = " + w.getRank() + " / " + w.getSize());
}

test();

