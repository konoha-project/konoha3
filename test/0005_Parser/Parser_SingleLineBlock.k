// written by kimio kuramitsu

int f(int n) {
	int c = 1;
	if(n == 1) c = n;
	c = c + 1;
	return c;
}

//System.p("a="+ f(1));
assert(f(1) == 2);

