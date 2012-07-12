// written by shunsuke shida

K.import("konoha");

class A {
       int x;
       @Public Int f() {return x+1;}
}

class B  extends A  {
	int y;
	B() {
		x = 1;
		y = 2;
	}
	@Public Int g() {return x+y;}
}

B b = new B();
assert(b.x == 1);
assert(b.y == 2);
assert(b.f() == 2);
assert(b.g() == 3);
