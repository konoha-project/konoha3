//written by Joseph

K.import("konoha.regexp");
K.import("konoha.assign");
K.import("konoha.class");

void test() {
	//check whether or not "i" modifier is set.
	RegExp patt = /世界/i;
	assert(patt.ignoreCase == true);
	patt = /世界/;
	assert(patt.ignoreCase == false);
}

test();
