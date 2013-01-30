import("Type.Json");
import("Type.Float");
import("JavaScript.Array");
import("JavaStyle.Object");
import("Syntax.Null");

void test_string() {
	Json j = new Json(String);
	assert(j.toString() == "\"\"");
}

void test_int() {
	Json j = new Json(int);
	assert(j.toString() == "0");
}

void test_float() {
	Json j = new Json(float);
	assert(j.tofloat() - FLOAT_EPSILON < 0.00001);
}

void test_boolean() {
	Json j = new Json(boolean);
	assert(j.toString() == "false");
}

void test_array() {
	Json j = new Json([]);
	assert(j.toString() == "[]");
}

void test_object() {
	Json j = new Json(Object);
	assert(j.toString() == "{}");
}

void test_null() {
	Json j = new Json(null);
	assert(j.toString() == "{}");
}

test_string();
test_int();
test_float();
test_boolean();
test_array();
test_object();
test_null();
