// written by okamoto

import("JavaScript.Date");

void test() {
	Date d = new Date(2012, 9, 10, 18, 46, 30, 10);
	assert(d.getUTCDate() == 10);
	assert(d.getUTCDay() == 3);
	assert(d.getUTCFullYear() == 2012);
	assert(d.getUTCHours() == 9);
	assert(d.getUTCMilliseconds() == 10);
	assert(d.getUTCMinutes() == 46);
	assert(d.getUTCMonth() >= 9);
	assert(d.getUTCSeconds() >= 30);
}

test();
