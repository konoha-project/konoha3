// written by kimio kuramitsu

//import("konoha.new"); Don't use this
import("Syntax.JavaStyleClass");

class C {
	int x = 1;
	int y = 2;
	int z = 3;
	String name = "naruto";
	int age = 10;
	String friend = "nakata";
	boolean isReally = false;
	int w = 10000;
	int yes = 1;
	String school="ynu";
	C() {}
}

void test() {
	C c = new C();
	assert(c.age == 10);
	assert(c.x == 1);
	assert(c.school== "ynu");
	assert(c.friend == "nakata");
	assert(c.z == 3);
}

test();

