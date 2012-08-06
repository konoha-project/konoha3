// written by shinpei

K.import("konoha.iterator")

void iteratorTest () {
	String str = "hello,world";
	Iterator[String] itr = str.toIterator();
	while (itr.hasNext()) {
		p(itr.next());
	}
}

iteratorTest();

