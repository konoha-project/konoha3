//reported by okamoto
//bug732

void test(String str) {
    System.p(str);
}

void main() {
    Func[void, String] f = test;
    f("called");
}

main();
