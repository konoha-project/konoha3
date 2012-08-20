// written by okamoto
K.import("konoha");
K.import("konoha.date");

void f() {
	Date d = new Date();
	p(d.getDate() > 0);
	p(d.getDay() > 0);
	p(d.getFullYear() >= 2012);
	p(d.getHours() >= 0);
	p(d.getMilliseconds() >= 0);
	p(d.getMinutes() >= 0);
	p(d.getMonth() >= 0);
	p(d.getSeconds() >= 0);
	p(d.getTime() > 0);
	p(d.getTimezoneOffset() > -720 || d.getTimezoneOffset() < 720);
	p(d.getYear() >= 112);
}

f();
