import("Syntax.Lisp");

void f() { 
    assert((+ 0)==0);
    assert((+ 0 1)==1);
    assert((+ 0 1 2)==3);
    assert((+ 0 1 2 3)==6);
    assert((+ 0 1 2 3 4)==10);
    assert((+ 0 1 2 3 4 5)==15);
    assert((+ 0 1 2 3 4 5 6)==21);
    assert((+ 0 1 2 3 4 5 6 7)==28);
    assert((+ 0 1 2 3 4 5 6 7 8)==36);
    assert((+ 0 1 2 3 4 5 6 7 8 9)==45);
    assert((+ 0 1 2 3 4 5 6 7 8 9 10)==55);
}

f();
