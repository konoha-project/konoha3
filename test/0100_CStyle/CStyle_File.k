// written by kimio kuramitsu

import("cstyle.file");

void test() {  // just check if the functions are available
	FILE fp = fopen("/etc/hosts", "r");
	fclose(fp);
}
test();

