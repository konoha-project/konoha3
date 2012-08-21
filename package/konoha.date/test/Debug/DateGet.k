// written by okamoto
K.import("konoha.assign");
K.import("konoha.class");
K.import("konoha.date");

void f() {
	Date d = new Date();
	assert(d.getDate() > 0);
	assert(d.getDay() > 0);
	assert(d.getFullYear() >= 2012);
	assert(d.getHours() >= 0);
	assert(d.getMilliseconds() >= 0);
	assert(d.getMinutes() >= 0);
	assert(d.getMonth() >= 0);
	assert(d.getSeconds() >= 0);
	assert(d.getTime() > 0);
	assert(d.getTimezoneOffset() > -720 || d.getTimezoneOffset() < 720);
	assert(d.getYear() >= 112);
}

f();
