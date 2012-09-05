K.import("mecab");
K.import("konoha.regexp");
K.import("konoha");

void main() {
	Tagger m = new Tagger();
	String input = "太郎は次郎が持っている本を花子に渡した。";
	p("Input: " + input + "\n");
	String result = m.parse(input);
	String[] strs = result.split(/\n/);
	int i = 0;
	while(i < strs.getSize()) {
		String line = strs[i];
		if (line.search(/名詞/) > -1) {
			String[] noun = line.split(/[ \t]+/);
			p(noun[0]);
		}
		i += 1;
	}
}

main();
