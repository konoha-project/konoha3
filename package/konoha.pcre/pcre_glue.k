K.import("sugar");
K.import("konoha.while");
K.import("konoha.string");
K.import("konoha.assignment");
K.import("konoha.array");
K.import("konoha.class");

int regexToken(Token tk, String source) {
    if (source.get(1) == "/" || source.get(1) == "*") {
        return 0;
    }
    int i = 1;
    int len = source.getSize();
    while (i < len) {
        if (source.get(i) == "\n") {
            "* must close with '/' *"
            return 0;
        }
        if (source.get(i) == "/") {
            int end  = i + 1;
            int pos = end;
            if (end < len) {
                if (source.get(end) == "i") {
                    end = end + 1;
                }
            }
            String reg = source.substring(1, end-2);
            String opt = source.substring(pos, end);
            if (pos == end) {
                "FIXME: String.substring(0, 0) is not return empty string";
                opt = "";
            }
            String[] regs = new String[2];
            regs.set(0, reg);
            regs.set(1, opt);
            tk.setKeyword("$regex");
            tk.setSubArray(regs);
            return end;
        }
        i = i + 1;
    }
    System.p("source="+source);
    return 0;
}

K.addTokenizeFunc("/", regexToken);
