// written by kimio kuramitsu

K.import("konoha");

void test() {
  int[] a = new int[0];
  a.add(1);
  a.add(2);
  a.add(9);
  assert(a.getSize() == 3);
  assert(a.get(0) == 1);
  assert(a.get(1) == 2);
  assert(a.get(2) == 9);
}


test();

