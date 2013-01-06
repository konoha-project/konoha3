//written by takeo onoda

import("Java.String");

void test() {
	assert("  \t\n\t foo \t\n bar \t\n\t".trim() == "foo \t\n bar");
	assert("  \t\n\t aあi \t\n いuう \t\n\t".trim() == "aあi \t\n いuう");
	assert(" \t\n\t ".trim() == "");
}

test();
