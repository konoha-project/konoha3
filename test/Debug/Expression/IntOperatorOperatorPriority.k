/* this test is written by shinpei nakata */


void int_operator_operatorPriority_operatorPriority() { 
	int i2 = 2;
	int i3 = 3;
	int i4 = 4;
	System.p(i2 + i3 - i4);
	System.p(i2 + i3 * i4);
	System.p(i2 + i3 / i4);
	System.p(i2 + i3 % i4);
	System.p(i2 - i3 + i4);
	System.p(i2 - i3 * i4);
	System.p(i2 - i3 / i4);
	System.p(i2 - i3 % i4);
	System.p(i2 * i3 + i4);
	System.p(i2 * i3 - i4);
	System.p(i2 * i3 / i4);
	System.p(i2 * i3 % i4);
	System.p(i2 % i3 + i4);
	System.p(i2 % i3 - i4);
	System.p(i2 % i3 * i4);
	System.p(i2 % i3 / i4);
}
int_operator_operatorPriority_operatorPriority();

