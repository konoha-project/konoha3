// written by okamoto

import("JavaScript.Date");

void test() {
	Date d = new Date(2012, 9, 10, 18, 46, 30, 10);
	assert(d.getDate() == 10);
	assert(d.getDay() == 3);
	assert(d.getFullYear() == 2012);
	assert(d.getHours() == 18);
	assert(d.getMilliseconds() == 10);
	assert(d.getMinutes() == 46);
	assert(d.getMonth() == 9);
	assert(d.getSeconds() == 30);
	assert(d.getTime() == 1349862390010);
	assert(d.getTimezoneOffset() > -720 || d.getTimezoneOffset() < 720);
	assert(d.getYear() == 112);
}

test();
