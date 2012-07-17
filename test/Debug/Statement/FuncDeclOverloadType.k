// written by kimio kuramitsu

/**
int succ(int x) {
	return x + 1;
}
**/

String succ(String x) {
	return x + 1;
}

System.p(succ("1"));

void test() {
	assert(succ(1) == 2);
	assert(succ("1") == "11");
}

test();

