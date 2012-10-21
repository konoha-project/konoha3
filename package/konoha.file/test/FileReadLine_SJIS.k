// written by kimio
import("cstyle");
import("konoha.null");
import("konoha.file");
void test() {
	String file = FILE.scriptPath("files/sjis_kanji.txt");
	FILE fp = System.fopen(file, "r");
	String line;
	while((line = fp.readLine()) != null) {
		System.p(line);
	}
	fp.close();
}
test();

