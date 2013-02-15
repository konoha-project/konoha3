import("JavaScript.Regexp");
import("JavaScript.Array");
import("JavaScript.String");
import("Type.File");
import("Type.Bytes");
import("Syntax.CStyleWhile");
import("MiniKonoha.NameSpace");

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

String replace(String path, String data, String pattern, String before, String after) {
    String tmp = data;
    RegExp r1 = new RegExp(pattern);
    while (true) {
        String[] res = tmp.match(r1);
        if (res.getSize() == 0) {
            break;
        }
        tmp = tmp.replace(res.get(0), res.get(0).replace(before, after));
    }
    return tmp;
}

void emit(String path, String data) {
    FILE file = new FILE(path, "w");
    file.print(data);
}

void main() {
    String[] args = SCRIPT_ARGV;
    int i = 1;
    String[] before = ["js4.array","js4.string","konoha.float","konoha.math", "js4.regexp", "js4.date", "konoha.bytes", "konoha.file",
        "konoha.complex", "curl", "libvirt", "gmp", "openssl", "mecab", "llvm", "apache", "konoha.json", "konoha.string",
        "java2.object", "konoha.prototype", "konoha.console", "konoha.desugar", "konoha.field", "konoha.i",
        "konoha.ns", "konoha.global", "konoha.class", "konoha.dynamic", "konoha.untyped", "konoha.var", "libxml.xmlreader",
        "mecab", "konoha.mpi", "openssl", "python", "konoha.sql", "konoha.defined", "konoha.import",
        "java2.string", "konoha.eval"];
    String[] after  = ["JavaScript.Array","JavaScript.String","Type.Float","JavaScript.Math", "JavaScript.Regexp", "JavaScript.Date", "Type.Bytes", "Type.File",
        "Type.Complex", "Lib.Curl", "Lib.Libvirt", "Lib.Gmp", "Lib.Openssl", "Lib.Mecab", "Lib.Llvm", "Lib.Apache", "Type.Json", "Java.String",
        "Java.Object", "JavaScript.Prototype", "MiniKonoha.Console", "MiniKonoha.Syntax", "MiniKonoha.Field", "MiniKonoha.Man",
        "MiniKonoha.NameSpace", "Syntax.GlobalVariable", "Syntax.JavaStyleClass", "Type.Dynamic", "Syntax.UntypedVariable", "Type.StaticVar", "Lib.Libxml2.Xmlreader",
        "Lib.Mecab", "Lib.Mpi", "Lib.Openssl", "Lib.Python", "MiniKonoha.Sql", "MiniKonoha.NameSpace", "Syntax.JavaImport", "Java.String", "MiniKonoha.Eval"];


    while (true) {
        if (i == args.getSize()) {
            break;
        }
        String data = readFile(args[i]);
        System.p("Fix " + args[i]);
        data = replace(args[i], data, "[a-zA-Z]+\\*\\)", "*)", " *)");
        data = replace(args[i], data, "if \\(", "if (", "if(");
        data = replace(args[i], data, "KDEFINE_PACKAGE\\* ", "KDEFINE_PACKAGE* ", "KDEFINE_PACKAGE *");
        data = replace(args[i], data, "KKMethodName", "KKMethodName", "KMethodName");
        data = replace(args[i], data, "NodeingErrorMessage", "NodeingErrorMessage", "BlockingErrorMessage");
        int j = 0;
        while (j < before.getSize()) {
            data = replace(args[i], data,
                    "KRequirePackage\\(\"" + before[j] + "\", trace\\)",
                    "KRequirePackage(\"" + before[j] + "\", trace)",
                    "KRequirePackage(\"" + after[j]  + "\", trace)");
            data = replace(args[i], data,
                    "KImportPackage\\(ns, \"" + before[j] + "\", trace\\)",
                    "KImportPackage(ns, \"" + before[j] + "\", trace)",
                    "KImportPackage(ns, \"" + after[j]  + "\", trace)");
            j = j + 1;
        }

        data = replace(args[i], data, "^static\tkbool_t", "static\tkbool_t", "static kbool_t");
        data = replace(args[i], data, "#include<", "#include<", "#include <");
        data = replace(args[i], data, "#include\"", "#include\"", "#include \"");
        data = replace(args[i], data, "while \\(", "while (", "while(");
        data = replace(args[i], data, "\n\t\n", "\n\t\n", "\n\n");
        data = replace(args[i], data, ";\t\n", ";\t\n", ";\n");
        data = replace(args[i], data, "; \n", "; \n", ";\n");

        data = data.replace("minikonoha", "konoha");

        emit(args[i], data);
        i = i + 1;
    }
}

main();
