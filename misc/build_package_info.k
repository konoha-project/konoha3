import("posix.fd");
import("posix.path");
import("Type.File");
import("Type.Bytes");
import("Type.Json");
import("JavaScript.String");
import("JavaScript.Array");
import("Syntax.StringInterpolation");
import("Syntax.CStyleWhile");
import("Syntax.CStyleFor");
import("Syntax.Null");
import("Syntax.Number");
import("Syntax.GlobalVariable");
import("MiniKonoha.Map");

boolean hasFile(String path) {
    int fd = System.open(path, 0);
    if (fd != -1) {
        System.close(fd);
    }
    return fd != -1;
}

String readFile(String path) {
    FILE file = new FILE(path, "rb");
    int len = 0;
    String tmp = "";
    while (true) {
        int ch = file.getc();
        if (ch == -1) {
            break;
        }
        len = len + 1;
    }
    file.close();

    if (len == 0) {
        return "";
    }

    Bytes buf = new Bytes(len);
    file = new FILE(path, "r");
    file.read(buf, 0, len);
    file.close();
    return new String(buf);
}

String desc(String docdir, String package) {
    String path = "${docdir}/ja.md";
    if (!hasFile(path)) { return "${package}-"; }
    File file = new File(path, "r");
    return file.readLine();
}

String Json.toCSV() {
    String package = this.getString("package");
    String version = this.getString("version");
    String status  = this.getString("status");
    String plat    = this.getString("platform");
    String score   = this.getString("score");
    String lang    = this.getString("lang");
    String desc    = this.getString("desc");
    String html    = this.getString("html");
    return "${package}, ${version}, ${status}, ${plat}, ${score}, ${lang}, ${desc}, ${html},";
}

File info     = new File("./doc/LibraryMetadata.csv", "w");
File keywords = new File("./doc/LibraryInversePair.csv", "w");

void emitPackageInfo(String docdir, String package) {
    String path = "${docdir}/metadata";
    if (!hasFile(path)) { return; }
    String data = readFile(path);
    String desc = desc(docdir, package);
    Json json = Json.parse(data);
    json.setString("desc", desc.split("-").get(1));
    json.setString("lang", "ja");
    json.setString("score", "-1");
    json.setString("html", "${package}.html");
    if (true/*FIXME json.getString("version") != "0.0"*/) {
        global.info.println(json.toCSV());
    }
}
Array[String] Array[String].uniq() {
    /* FIXME implement more efficient code */
    Map[String] map = new Map[String]();
    for (int i=0; i < this.length; i = i + 1) {
        map.set(this[i], this[i]);
    }
    return map.keys();
}

void emitPackageKeyWord(String docdir, String package) {
    String path = "${docdir}/index";
    if (!hasFile(path)) { return; }
    String[] data = readFile(path).split("\n").uniq();
    for (int i=0; i < data.length; i = i + 1) {
        if (data[i] != "") {
            global.keywords.println("${package}, ${data[i]}");
        }
    }
}

void emit(String path) {
    DIR d = System.opendir(path);
    String dirname = "";
    while ((dirname = d.readFileName()) != null) {
        if (dirname != "." && dirname != ".." && dirname != "CMakeLists.txt") {
            String basedir = path + dirname;
            String docdir = basedir + "/doc";
            emitPackageInfo(docdir, dirname);
            emitPackageKeyWord(docdir, dirname);
        }
    }
    d.close();
}

void main() {
    emit("src/package/");
    emit("src/package-devel/");
}

main();
