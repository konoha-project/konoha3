// written by shinpei

K.import("konoha");

class Human {
	int age;
	String name;

	Human(int a) {
		age = a;
	}
}

void f() {
	Human[] ha = new Human[2];
	ha[0] = new Human(17);
	ha[1] = new Human(22);
	p(ha[0].age);
	p(ha[1].age);

}

f();
