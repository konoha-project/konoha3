// written by okamoto
K.import("konoha");
K.import("konoha.date");

void f() {
	Date d = new Date();
	d.setDate(15);
	p(d.getDate());
	d.setFullYear(2012);
	p(d.getFullYear());
	d.setHours(12);
	p(d.getHours());
	d.setMilliseconds(500);
	p(d.getMilliseconds);
	d.setMinutes(30);
	p(d.getMinutes());
	d.setMonth(11);
	p(d.getMonth());
	d.setSeconds(30);
	p(d.getSeconds());
	d.setTime(1000000000);
	p(d.getTime());
	d.setYear(112);
	p(d.getYear());
}

f();
