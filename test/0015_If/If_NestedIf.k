// written by kimio kuramitsu

void f(int n) {
	if(n > 0) {
		if(n < 0) { 
			assert(false);  // never hit
		}
	}
	else {
		if(n > 0) {
			assert(false);  // never hit
		}
	}
}

f(0);
f(1);


