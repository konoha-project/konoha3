// written by okamoto
K.import("konoha");
K.import("konoha.string");
K.import("konoha.date");

void f() {
	Date d = new Date();
	p(d.toDateString().getSize() > 0);
	p(d.toGMTString().getSize() > 0);
//	p(d.toISOString().getSize() > 0);
	p(d.toLocaleDateString().getSize() > 0);
	p(d.toLocaleTimeString().getSize() > 0);
	p(d.toLocaleString().getSize() > 0);
}

f();
