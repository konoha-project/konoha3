import("Type.Json");

void main(){
	Json json = Json.parse("{\"n\": 1, \"s\": \"str\", \"b\":true, \"a\":[1,2,3], \"o\":{\"n\": 2} }");
	assert(json.getSize() == 5);
	assert(json.hasKey("n"));
	assert(json.hasKey("s"));
	assert(json.hasKey("b"));
	assert(json.hasKey("a"));
	assert(json.getInt("n") == 1);
	assert(json.getString("s") == "str");
	assert(json.getBoolean("b") == true);
	assert(json.get("a").getInt(0) == 1);
	assert(json.get("a").getInt(1) == 2);
	assert(json.get("a").getInt(2) == 3);
	assert(json.get("o").getInt("n") == 2);
	json.setString("s", "hoge");
	assert(json.getString("s") == "hoge");
	json.setInt("n", 9999);
	assert(json.getInt("n") == 9999);
}
main();

