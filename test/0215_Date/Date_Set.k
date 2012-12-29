// written by okamoto

import("JavaScript.Date");

void test() {
	Date d = new Date();
	d.setDate(15);
	assert(d.getDate() == 15);
	d.setFullYear(2012);
	assert(d.getFullYear() == 2012);
	d.setHours(12);
	assert(d.getHours() == 12);
	d.setMilliseconds(500);
	assert(d.getMilliseconds() == 500);
	d.setMinutes(30);
	assert(d.getMinutes() == 30);
	d.setMonth(11);
	assert(d.getMonth() == 11);
	d.setSeconds(30);
	assert(d.getSeconds() == 30);
	d.setTime(1000000000);
	assert(d.getTime() == 1000000000);
	d.setYear(112);
	assert(d.getYear() == 112);
}

test();
