/* this test is written by shinpei nakata */

void global_setup() { 
	import("konoha.float")
}
global_setup();

boolean float_operator_div_div() { 
	float f6000000 = 6.000000;
	float f3000000 = 3.000000;
	System.p( 5.000000  /  4.000000 );
	System.p( 5.000000  /  f6000000 );
	System.p( f6000000  /  5.000000 );
	System.p( f6000000  /  f3000000 );
}
float_operator_div_div();

