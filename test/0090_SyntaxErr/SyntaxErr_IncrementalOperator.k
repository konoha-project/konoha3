// reported by Atsushi Uchida
// import("konoha.assign"); Don't use any more

void f() {
    int i = 0;
    assert(i == 0);
    i++;
    assert(i == 1);
    i--;
    assert(i == 0);
}

f();
