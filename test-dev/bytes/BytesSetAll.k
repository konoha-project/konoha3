/* written by motoki yoan */

//import("konoha.new"); Don't use this
import("Type.Bytes");

void test() {
	int size = 8;
	int ascii_a = 97 // 'a'
	int ascii_b = 98 // 'b'
	int ascii_c = 99 // 'c'
	Bytes a = new Bytes(size);
	Bytes b = new Bytes(size);
	Bytes c = new Bytes(size);
	a.setAll(ascii_a);
	b.setAll(ascii_b);
	c.setAll(ascii_c);
	assert(a.get(0) == ascii_a);
	assert(b.get(0) == ascii_b);
	assert(c.get(0) == ascii_c);
}

test();
