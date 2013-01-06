// generated with genstrstrby kimio kuramitsu
import("Java.String")

void test() {
  String a = "a";
  String B = "B";
  assert(("Banana" == "Banana") == true);
  assert(("Banana" == "BANANA") == false);
  assert(("Banana" == a) == false);
  assert(("Banana" == B) == false);
  assert(("Banana" == B + a) == false);
  assert(("Banana" == "") == false);
  assert(("Banana" == "BananaBANANA") == false);
  assert(("BANANA" == "Banana") == false);
  assert(("BANANA" == "BANANA") == true);
  assert(("BANANA" == a) == false);
  assert(("BANANA" == B) == false);
  assert(("BANANA" == B + a) == false);
  assert(("BANANA" == "") == false);
  assert(("BANANA" == "BananaBANANA") == false);
  assert((a == "Banana") == false);
  assert((a == "BANANA") == false);
  assert((a == a) == true);
  assert((a == B) == false);
  assert((a == B + a) == false);
  assert((a == "") == false);
  assert((a == "BananaBANANA") == false);
  assert((B == "Banana") == false);
  assert((B == "BANANA") == false);
  assert((B == a) == false);
  assert((B == B) == true);
  assert((B == B + a) == false);
  assert((B == "") == false);
  assert((B == "BananaBANANA") == false);
  assert((B + a == "Banana") == false);
  assert((B + a == "BANANA") == false);
  assert((B + a == a) == false);
  assert((B + a == B) == false);
  assert((B + a == B + a) == true);
  assert((B + a == "") == false);
  assert((B + a == "BananaBANANA") == false);
  assert(("" == "Banana") == false);
  assert(("" == "BANANA") == false);
  assert(("" == a) == false);
  assert(("" == B) == false);
  assert(("" == B + a) == false);
  assert(("" == "") == true);
  assert(("" == "BananaBANANA") == false);
  assert(("BananaBANANA" == "Banana") == false);
  assert(("BananaBANANA" == "BANANA") == false);
  assert(("BananaBANANA" == a) == false);
  assert(("BananaBANANA" == B) == false);
  assert(("BananaBANANA" == B + a) == false);
  assert(("BananaBANANA" == "") == false);
  assert(("BananaBANANA" == "BananaBANANA") == true);
}
test();
