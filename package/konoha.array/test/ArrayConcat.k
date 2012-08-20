// written by shinpei

K.import("konoha.global");
K.import("konoha.assign");
K.import("konoha.array");

void test () {
	String[] sa1 = ["a", "b"];
	String[] sa2 = ["c", "d", "e"];
	String[] sa3 = ["f", "g"];

	String[] sa = sa1.concat(sa2, sa3);
	assert(sa.getSize() == 7);
	assert(sa[0] == "a");
	   
}

test();
