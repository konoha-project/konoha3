// written by kimio kuramitsu

int g(int a) {
	System.p("a=" + a);
	return a;
}

int f(int x) {
	return g(${int a = x; a+1;});
}

System.p(f(1));
assert(f(1)==2);
