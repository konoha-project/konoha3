import("konoha.new");
import("cstyle", "while");
import("konoha.int");
import("konoha.string");
import("konoha.array");
import("konoha.assign");
import("jansson");

void main() {
	String sample = "{}"

	Json parsed = Json.parse(sample);

	// Set Json
	parsed.set("int", [1,2,3,4]);
	parsed.set("string", ["foo", "bar"]);

	// Get Json
	int[] ia = parsed.getIntArray("int");
	String[] sa = parsed.getStringArray("string");
	int i = 0;
	while (i < ia.getSize()) {
		System.p(ia[i]);
		i += 1;
	}
	i = 0;
	while (i < sa.getSize()) {
		System.p(sa[i]);
		i += 1;
	}

}

main();
