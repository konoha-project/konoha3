// written by shinpei

K.import("konoha")
K.import("konoha.iterator")

void iteratorTest () {
	Array[int] ia = new int[3];
	ia.add(3);
	ia.add(4);
	ia.add(5);
	Iterator[int] itr = ia.toIterator();
	while (itr.hasNext()) {
		p(itr.next());
	}
}

iteratorTest();

