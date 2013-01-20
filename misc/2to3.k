Import("JavaScript.Array");
Import("JavaScript.String");
Import("Type.File");
Import("Type.Bytes");
Import("Syntax.CStyleWhile");
Import("MiniKonoha.NameSpace");

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

    Bytes buf = new Bytes(len+1);
    file = new FILE(path, "r");
    file.read(buf, 0, len);
    file.close();
    String res = new String(buf, 0, len);
    return res;
}

String replace(String path, String data, String pattern, String before, String after) {
    String tmp = data;
    RegExp r1 = new RegExp(pattern);
    while (true) {
        String[] res = tmp.match(r1);
        if (res.getSize() == 0) {
            break;
        }
        tmp = tmp.replace(res[0], res[0].replace(before, after));
    }
    return tmp;
}

void emit(String path, String data) {
    FILE file = new FILE(path, "w");
    file.print(data);
}

void main() {
    String[] before = ["js4.array","js4.string","konoha.float","konoha.math", "js4.regexp", "js4.date", "konoha.bytes", "konoha.file",
        "konoha.complex", "curl", "libvirt", "gmp", "openssl", "mecab", "llvm", "apache", "konoha.json", "konoha.string",
        "java2.object", "konoha.prototype", "konoha.console", "konoha.desugar", "konoha.field", "konoha.i",
        "konoha.ns", "konoha.global", "konoha.class", "konoha.dynamic", "konoha.untyped", "konoha.var", "libxml.xmlreader",
        "mecab", "konoha.mpi", "openssl", "python", "konoha.sql", "konoha.defined", "konoha.import",
        "java2.string", "konoha.eval"];
    String[] after  = ["JavaScript.Array","JavaScript.String","Type.Float","JavaScript.Math", "JavaScript.Regexp", "JavaScript.Date", "Type.Bytes", "Type.File",
        "Type.Complex", "Lib.Curl", "Lib.Libvirt", "Lib.Gmp", "Lib.Openssl", "Lib.Mecab", "Lib.Llvm", "Lib.Apache", "Type.Json", "Java.String",
        "Java.Object", "JavaScript.Prototype", "MiniKonoha.Console", "MiniKonoha.Syntax", "MiniKonoha.ObjectModel", "MiniKonoha.Man",
        "MiniKonoha.NameSpace", "Syntax.GlobalVariable", "Syntax.JavaStyleClass", "Type.Dynamic", "Syntax.UntypedVariable", "Type.StaticVar", "Lib.Libxml2.Xmlreader",
        "Lib.Mecab", "Lib.Mpi", "Lib.Openssl", "Lib.Python", "MiniKonoha.Sql", "MiniKonoha.NameSpace", "Syntax.JavaImport", "Java.String", "MiniKonoha.Eval"];


    before.add("konoha.map");
    after.add( "MiniKonoha.Map");

    before.add("cstyle\", \"null");
    after.add( "Syntax.Null");
    before.add("cstyle\", \"while");
    after.add( "Syntax.CStyleWhile");
    before.add("cstyle\", \"break");
    after.add( "Syntax.CStyleWhile");
    before.add("cstyle\", \"continue");
    after.add( "Syntax.CStyleWhile");
    before.add("cstyle\", \"for");
    after.add( "Syntax.CStyleFor");
    before.add("cstyle\", \"$forStmt");
    after.add( "Syntax.CStyleFor");
    before.add("cstyle\", \"$Number");
    after.add( "Syntax.Number");

    String[] args = SCRIPT_ARGV;
    int i = 1;
    while (true) {
        if (i == args.getSize()) {
            break;
        }
        String data = readFile(args[i]);
        System.p("Fix " + args[i]);
        data = replace(args[i], data, "import \\(", "import (", "import(");
        data = replace(args[i], data, "import\\( ", "import( ", "import(");
        int j = 0;
        while (j < before.getSize()) {
            data = replace(args[i], data, "import\\(\"" + before[j] + "\"\\)", "import(\""  + before[j] + "\")", "import(\"" + after[j] + "\")");
            j = j + 1;
        }
        emit(args[i], data);
        i = i + 1;
    }
}

main();
