// written by shinpei

K.import("konoha")
K.import("konoha.iterator")

void iteratorTest () {
	String str = "hello,world";
	Iterator[String] itr = str.toIterator();
	int i = 0;
	while (itr.hasNext()) {
		p(itr.next());
	}
}

iteratorTest();

