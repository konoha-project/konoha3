K.import("konoha");

class A0 {
  int i0;
}

class A1 extends A0 {
  int i1;
  A0 o0;
}

class A2 extends A1 {
  int i2;
  A1 o1;
}

class A3 extends A2 {
  int i3;
  A2 o2;
}

class A4 extends A3 {
  int i4;
  A3 o3;
}

class A5 extends A4 {
  int i5;
  A4 o4;
}

class A6 extends A5 {
  int i6;
  A5 o5;
}

class A7 extends A6 {
  int i7;
  A6 o6;
}

void f() {
  A0 a0 = new A0();
  A1 a1 = new A1();
  A0 m1_0 = a1.o0;
  A2 a2 = new A2();
  A0 m2_0 = a2.o0;
  A1 m2_1 = a2.o1;
  A3 a3 = new A3();
  A0 m3_0 = a3.o0;
  A1 m3_1 = a3.o1;
  A2 m3_2 = a3.o2;
  A4 a4 = new A4();
  A0 m4_0 = a4.o0;
  A1 m4_1 = a4.o1;
  A2 m4_2 = a4.o2;
  A3 m4_3 = a4.o3;
  A5 a5 = new A5();
  A0 m5_0 = a5.o0;
  A1 m5_1 = a5.o1;
  A2 m5_2 = a5.o2;
  A3 m5_3 = a5.o3;
  A4 m5_4 = a5.o4;
  A6 a6 = new A6();
  A0 m6_0 = a6.o0;
  A1 m6_1 = a6.o1;
  A2 m6_2 = a6.o2;
  A3 m6_3 = a6.o3;
  A4 m6_4 = a6.o4;
  A5 m6_5 = a6.o5;
  A7 a7 = new A7();
  A0 m7_0 = a7.o0;
  A1 m7_1 = a7.o1;
  A2 m7_2 = a7.o2;
  A3 m7_3 = a7.o3;
  A4 m7_4 = a7.o4;
  A5 m7_5 = a7.o5;
  A6 m7_6 = a7.o6;
}

f();
