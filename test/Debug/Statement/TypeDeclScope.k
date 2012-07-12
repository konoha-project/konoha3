// written by kimio

void f(int x) {
	int a = 1;
	if(x == 0) {
		int a = 2;
		if(x == 0) {
			int a = 3;
			assert(a == 3);
		}
		assert(a == 2);
	}
	assert(a==1);
}

f(0);

