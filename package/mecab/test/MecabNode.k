K.import("konoha");
K.import("mecab");
//K.import("konoha.float");

void main() {
	Tagger m = new Tagger();
	String input = "太郎は次郎が持っている本を花子に渡した。";
	p("Input: " + input);
	MecabNode node = m.parseToNode(input);
	int i = 0;
	//while (node != null) {
	while (i < 17) {
		p("Surface:\t"  + node.getSurface());
		p("Feature:\t"  + node.getFeature());
		p("Length:\t"   + node.getLength());
		p("RLength:\t"  + node.getRLength());
		p("ID:\t"       + node.getID());
		p("rcAttr:\t"   + node.getRCAttr());
		p("lcAttr:\t"   + node.getLCAttr());
		p("CharType:\t" + node.getCharType());
		p("Stat:\t"     + node.getStat());
		p("isBest:\t"   + node.isBest());
		//p("Alpha:\t"    + node.alpha());
		//p("Beta:\t"     + node.beta());
		//p("prob:\t"     + node.prob());
		p("wcost:\t "    + node.wcost());
		p("cost:\t"     + node.cost());
		node = node.next();
		i += 1;
	}
}

main();
