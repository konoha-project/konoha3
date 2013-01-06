// generated with genstrstrby kimio kuramitsu
import("Java.String")

void test() {
  String a = "a";
  String B = "B";
  assert(("Banana" + "Banana") == "BananaBanana");
  assert(("Banana" + "BANANA") == "BananaBANANA");
  assert(("Banana" + a) == "Bananaa");
  assert(("Banana" + B) == "BananaB");
  assert(("Banana" + B + a) == "BananaBa");
  assert(("Banana" + "") == "Banana");
  assert(("Banana" + "BananaBANANA") == "BananaBananaBANANA");
  assert(("BANANA" + "Banana") == "BANANABanana");
  assert(("BANANA" + "BANANA") == "BANANABANANA");
  assert(("BANANA" + a) == "BANANAa");
  assert(("BANANA" + B) == "BANANAB");
  assert(("BANANA" + B + a) == "BANANABa");
  assert(("BANANA" + "") == "BANANA");
  assert(("BANANA" + "BananaBANANA") == "BANANABananaBANANA");
  assert((a + "Banana") == "aBanana");
  assert((a + "BANANA") == "aBANANA");
  assert((a + a) == "aa");
  assert((a + B) == "aB");
  assert((a + B + a) == "aBa");
  assert((a + "") == "a");
  assert((a + "BananaBANANA") == "aBananaBANANA");
  assert((B + "Banana") == "BBanana");
  assert((B + "BANANA") == "BBANANA");
  assert((B + a) == "Ba");
  assert((B + B) == "BB");
  assert((B + B + a) == "BBa");
  assert((B + "") == "B");
  assert((B + "BananaBANANA") == "BBananaBANANA");
  assert((B + a + "Banana") == "BaBanana");
  assert((B + a + "BANANA") == "BaBANANA");
  assert((B + a + a) == "Baa");
  assert((B + a + B) == "BaB");
  assert((B + a + B + a) == "BaBa");
  assert((B + a + "") == "Ba");
  assert((B + a + "BananaBANANA") == "BaBananaBANANA");
  assert(("" + "Banana") == "Banana");
  assert(("" + "BANANA") == "BANANA");
  assert(("" + a) == "a");
  assert(("" + B) == "B");
  assert(("" + B + a) == "Ba");
  assert(("" + "") == "");
  assert(("" + "BananaBANANA") == "BananaBANANA");
  assert(("BananaBANANA" + "Banana") == "BananaBANANABanana");
  assert(("BananaBANANA" + "BANANA") == "BananaBANANABANANA");
  assert(("BananaBANANA" + a) == "BananaBANANAa");
  assert(("BananaBANANA" + B) == "BananaBANANAB");
  assert(("BananaBANANA" + B + a) == "BananaBANANABa");
  assert(("BananaBANANA" + "") == "BananaBANANA");
  assert(("BananaBANANA" + "BananaBANANA") == "BananaBANANABananaBANANA");
}
test();
