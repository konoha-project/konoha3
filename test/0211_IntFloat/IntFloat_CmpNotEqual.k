import("Type.Float")

void float_operator_int_cmpNotEqual_IntCmpNotEqual() { 
	int i1 = 1;
	float f2500000 = 2.500000;
	assert( 2.500000  !=  1  == true);
	assert( 2.500000  !=  i1  == true);
	assert( i1  !=  2.500000  == true);
	assert( i1  !=  f2500000  == true);
}

float_operator_int_cmpNotEqual_IntCmpNotEqual();
