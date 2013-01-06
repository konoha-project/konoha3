import("Type.Json");
import("Type.Float");

void test_string() {
	Json j = Json.parse("[]");
	j.add("foo");
	System.p(j);
}

void test_int() {
	Json j = Json.parse("[]");
	j.add(123);
	System.p(j);
}

void test_float() {
	Json j = Json.parse("[]");
	j.add(1.001);
	System.p(j);
}

void test_bool() {
	Json j = Json.parse("[]");
	j.add(true);
	System.p(j);
}

test_string();
test_int();
test_float();
test_bool();
