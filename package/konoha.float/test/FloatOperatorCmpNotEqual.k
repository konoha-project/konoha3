/* this test is written by shinpei nakata */

void global_setup() { 
	import("konoha.float")
}
global_setup();

boolean float_operator_cmpNotEqual_cmpNotEqual() { 
	float f1000000 = 1.000000;
	float f2000000 = 2.000000;
	System.p( 1.000000  !=  2.000000 );
	System.p( 1.000000  !=  f1000000 );
	System.p( f1000000  !=  1.000000 );
	System.p( f1000000  !=  f2000000 );
}
float_operator_cmpNotEqual_cmpNotEqual();

