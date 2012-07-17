// written by kimio kurmaitsu

int succ(int x) {
	return x;
}

System.p(succ(1));
assert(succ(1) == 1);

@Override int succ(int x) {
	return x+1;
}

System.p(succ(1));
assert(succ(1) == 2);

