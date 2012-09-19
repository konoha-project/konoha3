/* written by shinpei */
import("konoha.assign");
import("konoha.while");
import("konoha.string");
import("konoha.bytes");
import("posix.file");

void f () {
	FILE f = System.fopen("../../../package/posix.file/test/Debug/newfile.txt", "w");
	String contents = "hello world";
	int len = contents.getSize();
	f.write(contents.asBytes(), 0, len);
	f.close();
}

f();
		 
