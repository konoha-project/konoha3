import("JavaScript.Regexp");
import("JavaScript.Array");
import("JavaScript.String");
import("Type.File");
import("Type.Bytes");
import("Syntax.CStyleWhile");
import("Konoha.NameSpace");

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

    while (true) {
        if (i == args.getSize()) {
            break;
        }
        String data = readFile(args[i]);
        System.p("Fix " + args[i]);
        data = replace(args[i], data, "[a-zA-Z]+\\*\\)", "*)", " *)");
        data = replace(args[i], data, "if \\(", "if (", "if(");
        data = replace(args[i], data, "KDEFINE_PACKAGE\\* ", "KDEFINE_PACKAGE* ", "KDEFINE_PACKAGE *");
        data = replace(args[i], data, "#include<", "#include<", "#include <");
        data = replace(args[i], data, "#include\"", "#include\"", "#include \"");
        data = replace(args[i], data, "konoha3/konoha.h", "konoha3/konoha.h", "konoha3.h");
        data = replace(args[i], data, "#include \"konoha3/sugar.h\"", "#include \"konoha3/sugar.h\"", "");
        data = replace(args[i], data, "while \\(", "while (", "while(");
        //data = replace(args[i], data, "[^a-zA-Z]kNode", "kNode", "kUntypedNode");
        data = replace(args[i], data, "KType_Node", "KType_Node", "KType_UntypedNode");
        data = replace(args[i], data, "\n\t\n", "\n\t\n", "\n\n");
        data = replace(args[i], data, ";\t\n", ";\t\n", ";\n");
        data = replace(args[i], data, "; \n", "; \n", ";\n");

        data = data.replace("minikonoha", "konoha");

        emit(args[i], data);
        i = i + 1;
    }
}

main();
