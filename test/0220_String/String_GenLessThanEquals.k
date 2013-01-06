// generated with genstrstrby kimio kuramitsu
import("Java.String")

void test() {
  String a = "a";
  String B = "B";
  assert(("Banana" <= "Banana") == true);
  assert(("Banana" <= "BANANA") == false);
  assert(("Banana" <= a) == true);
  assert(("Banana" <= B) == false);
  assert(("Banana" <= B + a) == false);
  assert(("Banana" <= "") == false);
  assert(("Banana" <= "BananaBANANA") == true);
  assert(("BANANA" <= "Banana") == true);
  assert(("BANANA" <= "BANANA") == true);
  assert(("BANANA" <= a) == true);
  assert(("BANANA" <= B) == false);
  assert(("BANANA" <= B + a) == true);
  assert(("BANANA" <= "") == false);
  assert(("BANANA" <= "BananaBANANA") == true);
  assert((a <= "Banana") == false);
  assert((a <= "BANANA") == false);
  assert((a <= a) == true);
  assert((a <= B) == false);
  assert((a <= B + a) == false);
  assert((a <= "") == false);
  assert((a <= "BananaBANANA") == false);
  assert((B <= "Banana") == true);
  assert((B <= "BANANA") == true);
  assert((B <= a) == true);
  assert((B <= B) == true);
  assert((B <= B + a) == true);
  assert((B <= "") == false);
  assert((B <= "BananaBANANA") == true);
  assert((B + a <= "Banana") == true);
  assert((B + a <= "BANANA") == false);
  assert((B + a <= a) == true);
  assert((B + a <= B) == false);
  assert((B + a <= B + a) == true);
  assert((B + a <= "") == false);
  assert((B + a <= "BananaBANANA") == true);
  assert(("" <= "Banana") == true);
  assert(("" <= "BANANA") == true);
  assert(("" <= a) == true);
  assert(("" <= B) == true);
  assert(("" <= B + a) == true);
  assert(("" <= "") == true);
  assert(("" <= "BananaBANANA") == true);
  assert(("BananaBANANA" <= "Banana") == false);
  assert(("BananaBANANA" <= "BANANA") == false);
  assert(("BananaBANANA" <= a) == true);
  assert(("BananaBANANA" <= B) == false);
  assert(("BananaBANANA" <= B + a) == false);
  assert(("BananaBANANA" <= "") == false);
  assert(("BananaBANANA" <= "BananaBANANA") == true);
}
test();
