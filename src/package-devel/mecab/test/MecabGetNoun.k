import("Lib.Mecab");
import("JavaScript.Regexp");
import("konoha");

void main() {
	Tagger m = new Tagger();
	String input = "太郎は次郎が持っている本を花子に渡した。";
	System.p("Input: " + input + "\n");
	String result = m.parse(input);
	String[] strs = result.split(/\n/);
	int i = 0;
	while(i < strs.getSize()) {
		String line = strs[i];
		if(line.search(/名詞/) > -1) {
			String[] noun = line.split(/[ \t]+/);
			System.p(noun[0]);
		}
		i += 1;
	}
}

main();
