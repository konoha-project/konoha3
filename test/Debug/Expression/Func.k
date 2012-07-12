// written by kimio kuramitsu

int succ(int n) {
	return n+1;
}

void f() {
	Func[Int,Int] g = succ;
	assert(g(1) == 2);
}

f();

