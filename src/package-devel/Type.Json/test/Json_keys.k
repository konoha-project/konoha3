Import("Type.Json");
Import("JavaScript.Array");

void main(){
	Json json = Json.parse("{\"n\": 1 }");
	String[] result = json.keys();
	assert(result.getSize() == 1);
	assert(result[0] == "n");
}
main();

