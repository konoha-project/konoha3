// written by okamoto
K.import("konoha");
K.import("konoha.date");

void f() {
	Date d = new Date();
	p(d.getUTCDate() > 0);
	p(d.getUTCDay() > 0);
	p(d.getUTCFullYear() >= 2012);
	p(d.getUTCHours() >= 0);
	p(d.getUTCMilliseconds() >= 0);
	p(d.getUTCMinutes() >= 0);
	p(d.getUTCMonth() >= 0);
	p(d.getUTCSeconds() >= 0);
}

f();
