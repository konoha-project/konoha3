Import("JavaScript.Array");
Import("JavaScript.String");
Import("Type.File");
Import("Type.Bytes");
Import("Syntax.CStyleWhile");
Import("Konoha.NameSpace");

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

    String[] args = SCRIPT_ARGV;
    int i = 1;
    while (true) {
        if (i == args.getSize()) {
            break;
        }
        String data = readFile(args[i]);
        System.p("Fix " + args[i]);
        data = replace(args[i], data, "import \\(", "import (", "import(");
        data = replace(args[i], data, "MiniKonoha\\.", "MiniKonoha.", "Konoha.");
        data = replace(args[i], data, "import\\( ", "import( ", "import(");
        emit(args[i], data);
        i = i + 1;
    }
}

main();
