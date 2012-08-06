//K.import("konoha");
K.import("konoha.hashmap");

class Hoge {
	int x;
	int y;
	Hoge (int x0, int y0) {
		this.x = x0;
		this.y = y0;
	}
}

HashMap[Hoge] m = new HashMap[Hoge]();
Hoge h = new Hoge(1,2);
m.set("hoge", h);

Hoge h2 = m.get("hoge");
h.x = 3;
p(h2.x);
