// written by shinpei

import("konoha.assign");
import("konoha.while");

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
