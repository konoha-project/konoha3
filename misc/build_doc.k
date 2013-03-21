import("JavaScript.Regexp");
import("JavaScript.Array");
import("JavaScript.String");
import("Java.String");
import("Type.File");
import("Type.Bytes");
import("Syntax.CStyleWhile");
import("Syntax.CStyleFor");
import("Syntax.StringInterpolation");
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

void main() {
    String[] data = SCRIPT_ARGV;
    String comment = "";
    for (int i=1; i < data.length; i = i + 1) {
        String l = data[i];
        if (l == "") {
            break;
        }
        String p = l.split("/")[3];
        String output = "doc/${p}.html";
        stdout.println("pandoc -o ${p}.html5 ${l}");
        stdout.println("echo '<html>'  >  ${output}");
        stdout.println("echo '<head>'  >> ${output}");
        stdout.println("echo '</head>' >> ${output}");
        stdout.println("echo '<body>'  >> ${output}");
        stdout.println("cat ${p}.html5 >> ${output}");
        stdout.println("echo '</body>' >> ${output}");
        stdout.println("echo '</html>' >> ${output}");
        stdout.println("rm ${p}.html5");
    }
}

main();
