import("json");
import("konoha.array");
import("konoha.new");
import("konoha.string");

void String_toJson() {
    Json json = "{\"key\" : \"val\"}".toJson();
    assert(json.getString("key") == "val");
}
void Array_toJson() {
    int[] a = new int[4];
    a.set(0, 1);
    a.set(1, 2);
    a.set(2, 3);
    a.set(3, 4);
    Json json = a.toJson();
    //System.p(json.toString());
    //System.p(json);
    //System.p("[1,2,3,4]");
    assert(json.toString() == "[1,2,3,4]");
}

String_toJson();
Array_toJson();
