// generated with genstrstrby kimio kuramitsu
import("Java.String")

void test() {
  String a = "a";
  String B = "B";
  assert(("Banana").startsWith("Banana") == true);
  assert(("Banana").startsWith("BANANA") == false);
  assert(("Banana").startsWith(a) == false);
  assert(("Banana").startsWith(B) == true);
  assert(("Banana").startsWith("") == true);
  assert(("Banana").startsWith((B+a)) == true);
  assert(("Banana").startsWith("BananaBANANA") == false);
  assert(("BANANA").startsWith("Banana") == false);
  assert(("BANANA").startsWith("BANANA") == true);
  assert(("BANANA").startsWith(a) == false);
  assert(("BANANA").startsWith(B) == true);
  assert(("BANANA").startsWith("") == true);
  assert(("BANANA").startsWith((B+a)) == false);
  assert(("BANANA").startsWith("BananaBANANA") == false);
  assert((a).startsWith("Banana") == false);
  assert((a).startsWith("BANANA") == false);
  assert((a).startsWith(a) == true);
  assert((a).startsWith(B) == false);
  assert((a).startsWith("") == true);
  assert((a).startsWith((B+a)) == false);
  assert((a).startsWith("BananaBANANA") == false);
  assert((B).startsWith("Banana") == false);
  assert((B).startsWith("BANANA") == false);
  assert((B).startsWith(a) == false);
  assert((B).startsWith(B) == true);
  assert((B).startsWith("") == true);
  assert((B).startsWith((B+a)) == false);
  assert((B).startsWith("BananaBANANA") == false);
  assert(("").startsWith("Banana") == false);
  assert(("").startsWith("BANANA") == false);
  assert(("").startsWith(a) == false);
  assert(("").startsWith(B) == false);
  assert(("").startsWith("") == true);
  assert(("").startsWith((B+a)) == false);
  assert(("").startsWith("BananaBANANA") == false);
  assert(((B+a)).startsWith("Banana") == false);
  assert(((B+a)).startsWith("BANANA") == false);
  assert(((B+a)).startsWith(a) == false);
  assert(((B+a)).startsWith(B) == true);
  assert(((B+a)).startsWith("") == true);
  assert(((B+a)).startsWith((B+a)) == true);
  assert(((B+a)).startsWith("BananaBANANA") == false);
  assert(("BananaBANANA").startsWith("Banana") == true);
  assert(("BananaBANANA").startsWith("BANANA") == false);
  assert(("BananaBANANA").startsWith(a) == false);
  assert(("BananaBANANA").startsWith(B) == true);
  assert(("BananaBANANA").startsWith("") == true);
  assert(("BananaBANANA").startsWith((B+a)) == true);
  assert(("BananaBANANA").startsWith("BananaBANANA") == true);
}
test();
