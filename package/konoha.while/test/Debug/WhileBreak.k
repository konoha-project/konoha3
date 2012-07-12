// written by shinpei

K.import("konoha.assignment");
K.import("konoha.while");

int f () {
	int i;
	while (i < 100) {
		i = i + 1;
		if (i == 10) {
			break;
		}
	}
	return i;
}
assert(f() == 10);
