// written by imasahiro
import("hello_world");
import("konoha.new");

void f() {
    Person naruto = new Person("naruto", 17);
    String ret = naruto.say();
    assert(ret == "hello , naruto!");
}

f();
