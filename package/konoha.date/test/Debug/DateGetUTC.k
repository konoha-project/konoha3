// written by okamoto
K.import("konoha.assign");
K.import("konoha.class");
K.import("konoha.date");

void f() {
	Date d = new Date();
	assert(d.getUTCDate() > 0);
	assert(d.getUTCDay() > 0);
	assert(d.getUTCFullYear() >= 2012);
	assert(d.getUTCHours() >= 0);
	assert(d.getUTCMilliseconds() >= 0);
	assert(d.getUTCMinutes() >= 0);
	assert(d.getUTCMonth() >= 0);
	assert(d.getUTCSeconds() >= 0);
}

f();
