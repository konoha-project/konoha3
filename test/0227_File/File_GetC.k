// written by kimio

import("cstyle");
import("konoha.file");

void test() {
	String file = FILE.scriptPath("files/ascii.txt");
	FILE fp = new FILE(file, "r");
	int ch;
	while((ch = fp.getc()) != EOF) {
		stdout.putc(ch);
	}
	fp.close();
}
test();

