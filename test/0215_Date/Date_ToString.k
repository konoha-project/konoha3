// written by okamoto

import("JavaScript.String");
import("JavaScript.Regexp");
import("JavaScript.Date");

void test() {
	Date d = new Date();
	assert(d.toDateString().search(/[A-Z][a-z]*\s[A-Z][a-z]*\s\d{1,2}\s\d{4}/) == 0);
	assert(d.toGMTString().search(/[A-Z][a-z]*,\s\d{1,2}\s[A-Z][a-z]*\s\d{4}\s\d{2}:\d{2}:\d{2}\s[A-Z]*/) == 0);
	assert(d.toISOString().search(/\d{4}-\d{2}-\d{2}T\d{2}:\d{2}:\d{2}\.\d{3}Z/) == 0);
	assert(d.toLocaleDateString().search(/[A-Z][a-z]*\s[A-Z][a-z]*\s\d{1,2}\s\d{4}/) == 0);
	assert(d.toLocaleTimeString().search(/\d{2}:\d{2}:\d{2}/) == 0);
	assert(d.toLocaleString().search(/[A-Z][a-z]*\s[A-Z][a-z]*\s\d{1,2}\s\d{4}\s\d{2}:\d{2}:\d{2}\s\([A-Z]*\)/) == 0);
	assert(d.toString().search(/[A-Z][a-z]*\s[A-Z][a-z]*\s\d{1,2}\s\d{4}\s\d{2}:\d{2}:\d{2}\s\([A-Z]*\)/) == 0);
	assert(d.toTimeString().search(/\d{2}:\d{2}:\d{2}/) == 0);
	assert(d.toUTCString().search(/[A-Z][a-z]*,\s\d{1,2}\s[A-Z][a-z]*\s\d{4}\s\d{2}:\d{2}:\d{2}\s[A-Z]*/) == 0);
}

test();
