//written by Joseph

K.import("konoha.regexp");
K.import("konoha.assign");
K.import("konoha.string");

void test() {
	String str = "Every man in the world! Every woman on earth!";
	RegExp patt = /man/g;
	assert(str.replace(patt, "person") == "Every person in the world! Every woperson on earth!");
	patt = /(wo)?man/g;
	//change and recompile the regular expression "patt"
	patt.complile(patt);
	assert(str.replace(patt, "person") == "Every person in the world! Every person on earth!");
}

test();
