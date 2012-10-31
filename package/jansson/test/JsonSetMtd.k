import("konoha.new");
import("cstyle", "while");
import("konoha.int");
import("konoha.float");
import("konoha.string");
import("konoha.array");
import("konoha.assign");
import("jansson");

void main() {
	String sample = "{}"

	Json parsed = Json.parse(sample);

	// Set Json

	// int -> json
	parsed.set("int", 10);

	// float -> json
	parsed.set("float", 1.0);

	// String -> json
	parsed.set("string", "hoge");

	// Boolean -> json
	parsed.set("bool", true);

	// Json -> json
	Json json = new Json();
	json.set("foo", "bar");
	json.set("hoge", "fuga");
	parsed.set("json", json);

	// Json[] -> json
	Json[] ja = new Json[0];
	json = new Json();
	json.set("foo", "bar");
	ja.add(json);
	json = new Json();
	json.set("hoge", "fuga");
	ja.add(json);
	parsed.set("array", ja);

	// Get Json
	System.p(parsed.getInt("int"));
	System.p(parsed.getFloat("float"));
	System.p(parsed.getString("string"));
	System.p(parsed.getBool("bool"));
	System.p(parsed.getJson("json"));
	Json[] a = parsed.getArray("array");
	int i = 0;
	System.p(a.getSize());
	while (i < a.getSize()) {
		System.p(a[i]);
		i += 1;
	}
}

main();
