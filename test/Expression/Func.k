// written by kimio kuramitsu

int succ(int n) {
	return n+1;
}

void f() {
	Func[int,int] g = succ;
	assert(g(1) == 2);
}

f();

