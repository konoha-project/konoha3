// written by shinpei

import("konoha.iterator")
import("Syntax.CStyleWhile");

void iteratorTest () {
	String str = "hello,world";
	Iterator[String] itr = str.toIterator();
	while (itr.hasNext()) {
		System.p(itr.next());
	}
}

iteratorTest();

