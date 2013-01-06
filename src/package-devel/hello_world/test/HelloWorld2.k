// written by imasahiro
import("hello_world");
//import("konoha.new"); Don't use this

void f() {
    Person naruto = new Person("naruto", 17);
    String ret = naruto.say();
    assert(ret == "hello , naruto!");
}

f();
