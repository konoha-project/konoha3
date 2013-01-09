import("konoha.json");

void main(){
	Json json = Json.parse("{\"n\": 1, \"s\": \"str\", \"b\":true, \"a\":[1,2,3], \"o\":{\"n\": 2} }");
	String[] result = json.keys();
	assert(result[0] == "n");
	assert(result[1] == "s");
	assert(result[2] == "a");
	assert(result[3] == "b");
	assert(result[4] == "o");
}
main();

