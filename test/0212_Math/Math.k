import("JavaScript.Math");
import("Type.Float");
System.p(Math.abs(4));
System.p(Math.fabs(9.0));
System.p(Math.pow(3.0, 4.0));
System.p(Math.ldexp(3.0, 1));
System.p(Math.modf(3.0, 1.0));
System.p(Math.frexp(3.0, 1.0));
System.p(Math.fmod(3.0, 4.0));
System.p(Math.ceil(3.5));
System.p(Math.floor(3.0));
System.p(Math.sqrt(9.0));
System.p(Math.exp(4.0));
System.p(Math.log10(3.0));
System.p(Math.log(4.0));
System.p(Math.sin(3.0));
System.p(Math.cos(2.0));
System.p(Math.tan(4.0));
System.p(Math.asin(5.0));
System.p(Math.acos(3.0));
System.p(Math.atan(3.0));
System.p(Math.atan2(3.0, 5.0));
System.p(Math.sinh(2.0));
System.p(Math.cosh(3.0));
System.p(Math.tanh(4.0));
System.p(MATH_E);
System.p(MATH_LOG2E);
System.p(MATH_LOG10E);
System.p(MATH_LN2);
System.p(MATH_LN10);
System.p(MATH_PI);
System.p(MATH_SQRT2);

void f() {
    float r0 = Math.random();
    float r1 = Math.random();
    float r2 = Math.random();
    float r3 = Math.random();
    assert(0.0 <= r0 && r0 <= 1.0);
    assert(0.0 <= r1 && r1 <= 1.0);
    assert(0.0 <= r2 && r2 <= 1.0);
    assert(0.0 <= r3 && r3 <= 1.0);
}

f();
