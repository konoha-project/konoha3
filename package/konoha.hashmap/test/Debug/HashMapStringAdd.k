K.import("konoha.hashmap");
K.import("konoha.new");
K.import("konoha.assign");
void f() {
	HashMap[String] m = new HashMap[String]();
	m.set("hoge", "world");
	m.set("ohge", "best"); // same hcode
	p(m.get("hoge"));
	p(m.get("ohge"));
}

f();
