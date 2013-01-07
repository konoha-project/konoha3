// generated with genstrstrby kimio kuramitsu
import("Java.String")

void test() {
  String a = "a";
  String B = "B";
  assert(("Banana").lastIndexOf("Banana") == 0);
  assert(("Banana").lastIndexOf("BANANA") == -1);
  assert(("Banana").lastIndexOf(a) == 5);
  assert(("Banana").lastIndexOf(B) == 0);
  assert(("Banana").lastIndexOf("") == 6);
  assert(("Banana").lastIndexOf((B+a)) == 0);
  assert(("Banana").lastIndexOf("BananaBANANA") == -1);
  assert(("BANANA").lastIndexOf("Banana") == -1);
  assert(("BANANA").lastIndexOf("BANANA") == 0);
  assert(("BANANA").lastIndexOf(a) == -1);
  assert(("BANANA").lastIndexOf(B) == 0);
  assert(("BANANA").lastIndexOf("") == 6);
  assert(("BANANA").lastIndexOf((B+a)) == -1);
  assert(("BANANA").lastIndexOf("BananaBANANA") == -1);
  assert((a).lastIndexOf("Banana") == -1);
  assert((a).lastIndexOf("BANANA") == -1);
  assert((a).lastIndexOf(a) == 0);
  assert((a).lastIndexOf(B) == -1);
  assert((a).lastIndexOf("") == 1);
  assert((a).lastIndexOf((B+a)) == -1);
  assert((a).lastIndexOf("BananaBANANA") == -1);
  assert((B).lastIndexOf("Banana") == -1);
  assert((B).lastIndexOf("BANANA") == -1);
  assert((B).lastIndexOf(a) == -1);
  assert((B).lastIndexOf(B) == 0);
  assert((B).lastIndexOf("") == 1);
  assert((B).lastIndexOf((B+a)) == -1);
  assert((B).lastIndexOf("BananaBANANA") == -1);
  assert(("").lastIndexOf("Banana") == -1);
  assert(("").lastIndexOf("BANANA") == -1);
  assert(("").lastIndexOf(a) == -1);
  assert(("").lastIndexOf(B) == -1);
  assert(("").lastIndexOf("") == 0);
  assert(("").lastIndexOf((B+a)) == -1);
  assert(("").lastIndexOf("BananaBANANA") == -1);
  assert(((B+a)).lastIndexOf("Banana") == -1);
  assert(((B+a)).lastIndexOf("BANANA") == -1);
  assert(((B+a)).lastIndexOf(a) == 1);
  assert(((B+a)).lastIndexOf(B) == 0);
  assert(((B+a)).lastIndexOf("") == 2);
  assert(((B+a)).lastIndexOf((B+a)) == 0);
  assert(((B+a)).lastIndexOf("BananaBANANA") == -1);
  assert(("BananaBANANA").lastIndexOf("Banana") == 0);
  assert(("BananaBANANA").lastIndexOf("BANANA") == 6);
  assert(("BananaBANANA").lastIndexOf(a) == 5);
  assert(("BananaBANANA").lastIndexOf(B) == 6);
  assert(("BananaBANANA").lastIndexOf("") == 12);
  assert(("BananaBANANA").lastIndexOf((B+a)) == 0);
  assert(("BananaBANANA").lastIndexOf("BananaBANANA") == 0);
}
test();
