// written by shinpei

K.import("konoha.assign");
K.import("konoha.while");

int f () {
	int i = 0;
	int sum = 0;
	
	while (i < 10) {
		i = i + 1;
		if (i == 5) {
			continue;
		} else {
			sum = sum + i;
		}
	}
	return sum;
}

assert(f() == 50);
