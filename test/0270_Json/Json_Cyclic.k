// reported by Atsushi Uchida
import("Type.Json");

void main() {
    Json json = new Json();
    json.setString("a","b");
    json.set("json",json);
    System.p(json);
}

main();
