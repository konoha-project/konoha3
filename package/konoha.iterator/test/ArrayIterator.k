// written by shinpei
import("konoha.while");
import("konoha.new");
import("konoha.iterator");
import("konoha.array");

void iteratorTest () {
	Array[int] ia = [];
	ia.add(3);
	ia.add(4);
	ia.add(5);
	Iterator[int] itr = ia.toIterator();
	while (itr.hasNext()) {
		System.p(itr.next());
	}
}

iteratorTest();

