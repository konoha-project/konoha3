import("Syntax.JavaStyleClass")

class Derived;
class Base{
	@Public int a;
	@Public Derived d;
}
class Derived extends Base{
	@Public Derived(){}
}


void main(){
	Base b = new Derived();
	assert(b.a == 0);
}
main();

