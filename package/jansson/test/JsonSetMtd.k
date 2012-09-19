K.import("jansson");
K.import("konoha.float");
K.import("konoha");

void main() {
	String sample = "{}"

	Json parsed = Json.parse(sample);

	// Set Json

	// int -> json
	parsed.setInt("int", 10);

	// float -> json
	parsed.setFloat("float", 1.0);

	// String -> json
	parsed.setString("string", "hoge");

	// Boolean -> json
	parsed.setBool("bool", true);

	// Json -> json
	Json json = new Json();
	json.setString("foo", "bar");
	json.setString("hoge", "fuga");
	parsed.setJson("json", json);

	// Json[] -> json
	Json[] ja = new Json[0];
	json = new Json();
	json.setString("foo", "bar");
	ja.add(json);
	json = new Json();
	json.setString("hoge", "fuga");
	ja.add(json);
	parsed.setArray("array", ja);

	// Get Json
	p(parsed.getInt("int"));
	p(parsed.getFloat("float"));
	p(parsed.getString("string"));
	p(parsed.getBool("bool"));
	p(parsed.getJson("json"));
	Json[] a = parsed.getArray("array");
	int i = 0;
	p(a.getSize());
	while (i < a.getSize()) {
		p(a[i]);
		i += 1;
	}
}

main();
