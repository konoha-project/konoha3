// reported by Atsushi Uchida
Import("Syntax.JavaStyleClass");

class Foo {
    Foo(){}
    int bar(){
        return this._func(1);
    }
    int ok(int i){
        return i+1;
    }
    int _func(int i){
        return i+1;
    }
}

void main() {
    Foo foo = new Foo();
    assert(foo.bar() == 2);
    assert(foo.ok(1) == 2);
    assert(foo._func(1) == 2);
}

main();
