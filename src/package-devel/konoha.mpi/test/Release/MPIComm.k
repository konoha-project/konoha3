// written by wakamatsu
import("Lib.Mpi");

void test() {
	MPIComm w = MPIComm.getWorld();
	System.p("hello world. rank = " + w.getRank() + " / " + w.getSize());
}

test();

