// written by kimio kuramitsu

int f(int a) {
	if(a==1) {
		return 1;
	}
	else if(a == 2) { /*hoge*/
		return 2;
	}
	/*hoge*/
	else {
		return 0;
	}
}

assert(f(0) == 0);
assert(f(1) == 1);
assert(f(2) == 2);
