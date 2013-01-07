/* written by shinpei nakata */
//import("konoha.new"); Don't use this
import("Type.Bytes");

void test() {
	String str = "hello world";
	Bytes ba = str.toBytes();
	String newStr = new String(ba);
	assert(newStr == "hello world");
}

test();
