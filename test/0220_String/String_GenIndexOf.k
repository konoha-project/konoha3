// generated with genstrstrby kimio kuramitsu
import("Java.String")

void test() {
  String a = "a";
  String B = "B";
  assert(("Banana").indexOf("Banana") == 0);
  assert(("Banana").indexOf("BANANA") == -1);
  assert(("Banana").indexOf(a) == 1);
  assert(("Banana").indexOf(B) == 0);
  assert(("Banana").indexOf("") == 0);
  assert(("Banana").indexOf((B+a)) == 0);
  assert(("Banana").indexOf("BananaBANANA") == -1);
  assert(("BANANA").indexOf("Banana") == -1);
  assert(("BANANA").indexOf("BANANA") == 0);
  assert(("BANANA").indexOf(a) == -1);
  assert(("BANANA").indexOf(B) == 0);
  assert(("BANANA").indexOf("") == 0);
  assert(("BANANA").indexOf((B+a)) == -1);
  assert(("BANANA").indexOf("BananaBANANA") == -1);
  assert((a).indexOf("Banana") == -1);
  assert((a).indexOf("BANANA") == -1);
  assert((a).indexOf(a) == 0);
  assert((a).indexOf(B) == -1);
  assert((a).indexOf("") == 0);
  assert((a).indexOf((B+a)) == -1);
  assert((a).indexOf("BananaBANANA") == -1);
  assert((B).indexOf("Banana") == -1);
  assert((B).indexOf("BANANA") == -1);
  assert((B).indexOf(a) == -1);
  assert((B).indexOf(B) == 0);
  assert((B).indexOf("") == 0);
  assert((B).indexOf((B+a)) == -1);
  assert((B).indexOf("BananaBANANA") == -1);
  assert(("").indexOf("Banana") == -1);
  assert(("").indexOf("BANANA") == -1);
  assert(("").indexOf(a) == -1);
  assert(("").indexOf(B) == -1);
  assert(("").indexOf("") == 0);
  assert(("").indexOf((B+a)) == -1);
  assert(("").indexOf("BananaBANANA") == -1);
  assert(((B+a)).indexOf("Banana") == -1);
  assert(((B+a)).indexOf("BANANA") == -1);
  assert(((B+a)).indexOf(a) == 1);
  assert(((B+a)).indexOf(B) == 0);
  assert(((B+a)).indexOf("") == 0);
  assert(((B+a)).indexOf((B+a)) == 0);
  assert(((B+a)).indexOf("BananaBANANA") == -1);
  assert(("BananaBANANA").indexOf("Banana") == 0);
  assert(("BananaBANANA").indexOf("BANANA") == 6);
  assert(("BananaBANANA").indexOf(a) == 1);
  assert(("BananaBANANA").indexOf(B) == 0);
  assert(("BananaBANANA").indexOf("") == 0);
  assert(("BananaBANANA").indexOf((B+a)) == 0);
  assert(("BananaBANANA").indexOf("BananaBANANA") == 0);
}
test();
