import("Type.Bytes");
import("Type.File");

/* file */

@Public @Static FILE System.fopen(String path, String mode) {
	return new FILE(path, mode);
}

@Static void System.fclose(FILE fp) {
	fp.close();
}

@Static int System.fwrite(Bytes buf, int size, int nitems, FILE fp) {
	return fp.write(buf, 0, size * nitems);
}

@Static int System.fread(Bytes buf, int size, int nitems, FILE fp) {
	return fp.read(buf, 0, size * nitems);
}

@Static void System.fprintln(FILE fp, String text) {
	fp.println(msg);
}

@Static void System.fprintf(FILE fp, String text) {
	fp.print(msg);
}

UseStaticFunc(System);


