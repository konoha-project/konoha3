import("konoha.regexp");
import("konoha.new");
import("konoha.string");
import("konoha.foreach");
import("konoha.const");
import("konoha.array");
import("konoha.file");
import("cstyle");

const dest = "./minikonoha/";
String readFile(String path) {
    FILE file = new FILE(path, "r");
    int len = 0;
    String tmp = "";
    while (true) {
        int ch = file.getc();
        if (ch == -1) {
            break;
        }
        tmp = tmp + String.fromCharCode(ch);
        len = len + 1;
    }
    file.close();
    return tmp;
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
        data = replace(args[i], data, "[a-zA-Z]+\\*\\)", "*)", " *)");
        data = replace(args[i], data, "if \\(", "if (", "if(");
        data = replace(args[i], data, "while \\(", "while (", "while(");
        emit(args[i], data);
        i = i + 1;
    }
}

main();
