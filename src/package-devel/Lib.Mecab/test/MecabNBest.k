import("Lib.Mecab");
import("konoha");

void main() {
	Tagger m = new Tagger();
	String input = "今日もしないとね．";
	System.p("Input: " + input);
	System.p("\n" + m.parse(input));
	m.NBestParse(3, input);
	m.NBestInit(input);
	int i = 0;
	while (i < 3) {
		String str = m.NBestNext();
		System.p("N Best[" + i + "]:\n" + str);
		i += 1;
	}
}

main();
