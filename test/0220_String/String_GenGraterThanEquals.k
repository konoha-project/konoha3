// generated with genstrstrby kimio kuramitsu
import("Java.String")

void test() {
  String a = "a";
  String B = "B";
  assert(("Banana" >= "Banana") == true);
  assert(("Banana" >= "BANANA") == true);
  assert(("Banana" >= a) == false);
  assert(("Banana" >= B) == true);
  assert(("Banana" >= B + a) == true);
  assert(("Banana" >= "") == true);
  assert(("Banana" >= "BananaBANANA") == false);
  assert(("BANANA" >= "Banana") == false);
  assert(("BANANA" >= "BANANA") == true);
  assert(("BANANA" >= a) == false);
  assert(("BANANA" >= B) == true);
  assert(("BANANA" >= B + a) == false);
  assert(("BANANA" >= "") == true);
  assert(("BANANA" >= "BananaBANANA") == false);
  assert((a >= "Banana") == true);
  assert((a >= "BANANA") == true);
  assert((a >= a) == true);
  assert((a >= B) == true);
  assert((a >= B + a) == true);
  assert((a >= "") == true);
  assert((a >= "BananaBANANA") == true);
  assert((B >= "Banana") == false);
  assert((B >= "BANANA") == false);
  assert((B >= a) == false);
  assert((B >= B) == true);
  assert((B >= B + a) == false);
  assert((B >= "") == true);
  assert((B >= "BananaBANANA") == false);
  assert((B + a >= "Banana") == false);
  assert((B + a >= "BANANA") == true);
  assert((B + a >= a) == false);
  assert((B + a >= B) == true);
  assert((B + a >= B + a) == true);
  assert((B + a >= "") == true);
  assert((B + a >= "BananaBANANA") == false);
  assert(("" >= "Banana") == false);
  assert(("" >= "BANANA") == false);
  assert(("" >= a) == false);
  assert(("" >= B) == false);
  assert(("" >= B + a) == false);
  assert(("" >= "") == true);
  assert(("" >= "BananaBANANA") == false);
  assert(("BananaBANANA" >= "Banana") == true);
  assert(("BananaBANANA" >= "BANANA") == true);
  assert(("BananaBANANA" >= a) == false);
  assert(("BananaBANANA" >= B) == true);
  assert(("BananaBANANA" >= B + a) == true);
  assert(("BananaBANANA" >= "") == true);
  assert(("BananaBANANA" >= "BananaBANANA") == true);
}
test();
