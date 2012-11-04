// written by kimio
import("cstyle");
import("cstyle"); // konoha.null is obsolate; use cstyle
import("konoha.file");
void test() {
	String file = FILE.scriptPath("files/ascii.txt");
	FILE fp = new FILE(file, "r");
	String line;
	while((line = fp.readLine()) != null) {
		stdout.println(line);
	}
	fp.close();
}
test();

