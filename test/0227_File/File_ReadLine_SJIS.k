// written by kimio
import("Syntax.CStyleWhile");
import("Type.File");
import("Syntax.Null");
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

