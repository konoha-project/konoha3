import("konoha.new");
import("konoha.bytes");

void test() {
	Bytes ba = "hello world".toBytes();
	String str = new String(ba, "EUC-JP");
}
