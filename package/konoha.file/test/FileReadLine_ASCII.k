// written by kimio
import("cstyle");
import("konoha.null");
import("konoha.file");
void test() {
	String file = FILE.scriptPath("files/ascii.txt");
	FILE fp = System.fopen(file, "r");
	String line;
	while((line = fp.readLine()) != null) {
		stdout.println(line);
	}
	fp.close();
}
test();

