// generated with genstrstrby kimio kuramitsu
import("Java.String")

void test() {
  String a = "a";
  String B = "B";
  assert(("Banana" != "Banana") == false);
  assert(("Banana" != "BANANA") == true);
  assert(("Banana" != a) == true);
  assert(("Banana" != B) == true);
  assert(("Banana" != B + a) == true);
  assert(("Banana" != "") == true);
  assert(("Banana" != "BananaBANANA") == true);
  assert(("BANANA" != "Banana") == true);
  assert(("BANANA" != "BANANA") == false);
  assert(("BANANA" != a) == true);
  assert(("BANANA" != B) == true);
  assert(("BANANA" != B + a) == true);
  assert(("BANANA" != "") == true);
  assert(("BANANA" != "BananaBANANA") == true);
  assert((a != "Banana") == true);
  assert((a != "BANANA") == true);
  assert((a != a) == false);
  assert((a != B) == true);
  assert((a != B + a) == true);
  assert((a != "") == true);
  assert((a != "BananaBANANA") == true);
  assert((B != "Banana") == true);
  assert((B != "BANANA") == true);
  assert((B != a) == true);
  assert((B != B) == false);
  assert((B != B + a) == true);
  assert((B != "") == true);
  assert((B != "BananaBANANA") == true);
  assert((B + a != "Banana") == true);
  assert((B + a != "BANANA") == true);
  assert((B + a != a) == true);
  assert((B + a != B) == true);
  assert((B + a != B + a) == false);
  assert((B + a != "") == true);
  assert((B + a != "BananaBANANA") == true);
  assert(("" != "Banana") == true);
  assert(("" != "BANANA") == true);
  assert(("" != a) == true);
  assert(("" != B) == true);
  assert(("" != B + a) == true);
  assert(("" != "") == false);
  assert(("" != "BananaBANANA") == true);
  assert(("BananaBANANA" != "Banana") == true);
  assert(("BananaBANANA" != "BANANA") == true);
  assert(("BananaBANANA" != a) == true);
  assert(("BananaBANANA" != B) == true);
  assert(("BananaBANANA" != B + a) == true);
  assert(("BananaBANANA" != "") == true);
  assert(("BananaBANANA" != "BananaBANANA") == false);
}
test();
