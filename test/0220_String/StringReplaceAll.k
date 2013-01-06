import("Java.String")

void f() {
    String x = "あいうえああ";
    assert(x.replaceAll("あ", "x") == "xいうえxx");
    String y = "abcda";
    assert(y.replaceAll("a", "x") == "xbcdx");
    assert("ababab".replaceAll("a", "x") == "xbxbxb");
    assert("あいあい".replaceAll("あ", "x") == "xいxい");
}

f();
