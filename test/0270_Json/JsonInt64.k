// reported at ML667
import("Type.Json");

void main() {
    int i = 12345678901;
    Json json = new Json();
    json.setInt("int64",i);
    assert(json.getInt("int64") == i);
    assert(json.toString() == "{\"int64\":12345678901}");
}

main();
