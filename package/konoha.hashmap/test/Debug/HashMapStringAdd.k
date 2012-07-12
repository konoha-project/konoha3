K.import("konoha.hashmap");
K.import("konoha");

HashMap[String] m = new HashMap[String]();
m.set("hoge", "world");
m.set("ohge", "best"); // same hcode

p(m.get("hoge"));
p(m.get("ohge"));
