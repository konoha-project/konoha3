import("konoha.new");
import("Type.Bytes");

void test() {
	Bytes ba = "hello world".toBytes();
	String str = new String(ba, "EUC-JP");
}
