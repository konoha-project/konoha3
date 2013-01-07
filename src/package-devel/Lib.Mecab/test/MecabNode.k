import("konoha");
import("Lib.Mecab");
//import("Type.Float");

void main() {
	Tagger m = new Tagger();
	String input = "太郎は次郎が持っている本を花子に渡した。";
	System.p("Input: " + input);
	MecabNode node = m.parseToNode(input);
	int i = 0;
	//while (node != null) {
	while (i < 17) {
		System.p("Surface:\t"  + node.getSurface());
		System.p("Feature:\t"  + node.getFeature());
		System.p("Length:\t"   + node.getLength());
		System.p("RLength:\t"  + node.getRLength());
		System.p("ID:\t"       + node.getID());
		System.p("rcAttr:\t"   + node.getRCAttr());
		System.p("lcAttr:\t"   + node.getLCAttr());
		System.p("CharType:\t" + node.getCharType());
		System.p("Stat:\t"     + node.getStat());
		System.p("isBest:\t"   + node.isBest());
		//System.p("Alpha:\t"    + node.alpha());
		//System.p("Beta:\t"     + node.beta());
		//System.p("prob:\t"     + node.prob());
		System.p("wcost:\t "    + node.wcost());
		System.p("cost:\t"     + node.cost());
		node = node.next();
		i += 1;
	}
}

main();
