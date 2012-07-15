/* written by shinpei */
K.import("konoha.assignment");
K.import("konoha.while");
K.import("konoha.string");
K.import("konoha.bytes");
K.import("posix.file");

void f () {
	FILE f = System.fopen("../../../package/posix.file/test/Debug/newfile.txt", "w");
	String contents = "hello world";
	int len = contents.getSize();
	f.write(contents.asBytes(), 0, len);
	f.close();
}

f();
		 
