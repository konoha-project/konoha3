// written by wakamatsu
import("konoha");
import("Type.Bytes");
import("Java.String");
import("Lib.Mpi");

void test_bytes() {
	Bytes b = "hello".asBytes();
	MPIData d = MPIData.fromBytes(b);
	Bytes b2 = d.toBytes();
	p(b2.asString());
}

void test_farray() {
	MPIData d = MPIData.newFloatArray(10);
	int i = 0;
	while(i < 10) {
		d.setf(i, 0.0 + i);
		i=i+1;
	}
	p("size = " + d.getSize());
	p("offset = " + d.getOffset());
	p("[5] = " + d.getf(5));
	d.setOffset(6);
	p("size = " + d.getSize());
	p("offset = " + d.getOffset());
	p("[0] = " + d.getf(0));
}

void test_iarray() {
	MPIData d = MPIData.newIntArray(10);
	int i = 0;
	while(i < 10) {
		d.seti(i, i);
		i=i+1;
	}
	p("size = " + d.getSize());
	p("offset = " + d.getOffset());
	p("[5] = " + d.geti(5));
	d.setOffset(6);
	p("size = " + d.getSize());
	p("offset = " + d.getOffset());
	p("[0] = " + d.geti(0));
}

test_bytes();
test_farray();
test_iarray();

