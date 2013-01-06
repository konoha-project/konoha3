//written by Joseph

import("JavaScript.Array");
import("JavaScript.Regexp");

void test() {
	String[] ma = "The rain in SPAIN stays mainly in the plain".match(/ain/g);
	assert(ma.getSize() == 3);
	assert(ma[0] == "ain");
	assert(ma[1] == "ain");
	assert(ma[2] == "ain");
	ma = "The rain in SPAIN stays mainly in the plain".match(/ain/gi);
	assert(ma.getSize() == 4);
	assert(ma[0] == "ain");
	assert(ma[1] == "AIN");
	assert(ma[2] == "ain");
	assert(ma[3] == "ain");
	RegExp patt = /[A-C]/gi;
	ma = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefgijklmnopqrstuvwxyz".match(patt);
	assert(ma.getSize() == 6);
	assert(ma[0] == "A");
	assert(ma[1] == "B");
	assert(ma[2] == "C");
	assert(ma[3] == "a");
	assert(ma[4] == "b");
	assert(ma[5] == "c");
	ma = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefgijklmnopqrstuvwxyz".match(/[A-C]/g);
	assert(ma.getSize() == 3);
	assert(ma[0] == "A");
	assert(ma[1] == "B");
	assert(ma[2] == "C");
}

test();
