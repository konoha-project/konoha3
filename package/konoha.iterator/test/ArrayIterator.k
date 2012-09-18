// written by shinpei
K.import("konoha.while");
K.import("konoha.new");
K.import("konoha.iterator");
K.import("konoha.array");

void iteratorTest () {
	Array[int] ia = [];
	ia.add(3);
	ia.add(4);
	ia.add(5);
	Iterator[int] itr = ia.toIterator();
	while (itr.hasNext()) {
		p(itr.next());
	}
}

iteratorTest();

