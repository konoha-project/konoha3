// written by kimio
import("cstyle");
import("konoha.null");
import("konoha.file");
void test() {
	String file = FILE.scriptPath("files/sjis_kanji.txt");
	FILE fp = System.fopen(file, "r");
	fp.setReaderCharset("SJIS");
	String line;
	while((line = fp.readLine()) != null) {
		stdout.println(line);
	}
	fp.close();
}
test();

