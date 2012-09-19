/* this test is written by shinpei nakata */

void global_setup() { 
	import("konoha.float")
}
global_setup();

void float_operator_int_cmpMoreThan_IntCmpMoreThan() { 
	int i1 = 1;
	float f2500000 = 2.500000;
	System.p( 2.500000  >  1 );
	System.p( 2.500000  >  i1 );
	System.p( i1  >  2.500000 );
	System.p( i1  >  f2500000 );
}
float_operator_int_cmpMoreThan_IntCmpMoreThan();

