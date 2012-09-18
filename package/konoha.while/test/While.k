// written by kimio

K.import("konoha.assign");
K.import("konoha.while");

int f() {
	int i;
	while(i < 10) {
		i = i + 1;
	}
	return i;
}

assert(f() == 10);

void test() {
    int i;
    while(i < 10) {
        i = i + 1;
        System.p(""+i);
    }
}

test();
