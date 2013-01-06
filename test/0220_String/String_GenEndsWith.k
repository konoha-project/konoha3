// generated with genstrstrby kimio kuramitsu
import("Java.String")

void test() {
  String a = "a";
  String B = "B";
  assert(("Banana").endsWith("Banana") == true);
  assert(("Banana").endsWith("BANANA") == false);
  assert(("Banana").endsWith(a) == true);
  assert(("Banana").endsWith(B) == false);
  assert(("Banana").endsWith("") == true);
  assert(("Banana").endsWith((B+a)) == false);
  assert(("Banana").endsWith("BananaBANANA") == false);
  assert(("BANANA").endsWith("Banana") == false);
  assert(("BANANA").endsWith("BANANA") == true);
  assert(("BANANA").endsWith(a) == false);
  assert(("BANANA").endsWith(B) == false);
  assert(("BANANA").endsWith("") == true);
  assert(("BANANA").endsWith((B+a)) == false);
  assert(("BANANA").endsWith("BananaBANANA") == false);
  assert((a).endsWith("Banana") == false);
  assert((a).endsWith("BANANA") == false);
  assert((a).endsWith(a) == true);
  assert((a).endsWith(B) == false);
  assert((a).endsWith("") == true);
  assert((a).endsWith((B+a)) == false);
  assert((a).endsWith("BananaBANANA") == false);
  assert((B).endsWith("Banana") == false);
  assert((B).endsWith("BANANA") == false);
  assert((B).endsWith(a) == false);
  assert((B).endsWith(B) == true);
  assert((B).endsWith("") == true);
  assert((B).endsWith((B+a)) == false);
  assert((B).endsWith("BananaBANANA") == false);
  assert(("").endsWith("Banana") == false);
  assert(("").endsWith("BANANA") == false);
  assert(("").endsWith(a) == false);
  assert(("").endsWith(B) == false);
  assert(("").endsWith("") == true);
  assert(("").endsWith((B+a)) == false);
  assert(("").endsWith("BananaBANANA") == false);
  assert(((B+a)).endsWith("Banana") == false);
  assert(((B+a)).endsWith("BANANA") == false);
  assert(((B+a)).endsWith(a) == true);
  assert(((B+a)).endsWith(B) == false);
  assert(((B+a)).endsWith("") == true);
  assert(((B+a)).endsWith((B+a)) == true);
  assert(((B+a)).endsWith("BananaBANANA") == false);
  assert(("BananaBANANA").endsWith("Banana") == false);
  assert(("BananaBANANA").endsWith("BANANA") == true);
  assert(("BananaBANANA").endsWith(a) == false);
  assert(("BananaBANANA").endsWith(B) == false);
  assert(("BananaBANANA").endsWith("") == true);
  assert(("BananaBANANA").endsWith((B+a)) == false);
  assert(("BananaBANANA").endsWith("BananaBANANA") == true);
}
test();
