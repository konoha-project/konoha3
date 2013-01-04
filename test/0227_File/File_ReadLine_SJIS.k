// written by kimio
import("cstyle");
import("cstyle"); // konoha.null is obsolate; use cstyle
import("Type.File");
void test() {
	String file = FILE.scriptPath("files/sjis_kanji.txt");
	FILE fp = new FILE(file, "r");
	fp.setReaderCharset("SJIS");
	String line;
	while((line = fp.readLine()) != null) {
		stdout.println(line);
	}
	fp.close();
}
test();

