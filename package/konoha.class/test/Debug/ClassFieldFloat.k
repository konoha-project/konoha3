// written by shinpei

K.import("konoha.float");
K.import("konoha");

class Human {
	float height;
}

void f() {
	Human h = new Human();
	h.height = 175;
	assert(h.height == 175.0);
}

f();
	
