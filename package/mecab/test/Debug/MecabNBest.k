K.import("mecab");
K.import("konoha");

void main() {
	Tagger m = new Tagger();
	String input = "今日もしないとね．";
	p("Input: " + input);
	p("\n" + m.parse(input));
	m.NBestParse(3, input);
	m.NBestInit(input);
	int i = 0;
	while (i < 3) {
		String str = m.NBestNext();
		p("N Best[" + i + "]:\n" + str);
		i += 1;
	}
}

main();
