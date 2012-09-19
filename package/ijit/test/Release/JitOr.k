// written by imasahiro
import("ijit");

boolean or_1(boolean b1, boolean b2, boolean b3, boolean b4) {
    return b1 || b2;
}
boolean or_2(boolean b1, boolean b2, boolean b3, boolean b4) {
    return b1 || b2 || b3;
}
boolean or_3(boolean b1, boolean b2, boolean b3, boolean b4) {
    return b1 || (b2 || b3);
}
boolean or_4(boolean b1, boolean b2, boolean b3, boolean b4) {
    return b1 || (b2 || b3) || b4;
}

System.p(or_1(true, true, true, true));
System.p(or_2(true, true, true, true));
System.p(or_3(true, true, true, true));
System.p(or_4(true, true, true, true));
