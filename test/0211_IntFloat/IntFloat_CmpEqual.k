import("Type.Float")

void float_operator_int_cmpEqual_IntCmpEqual() { 
	int i1 = 1;
	float f2500000 = 2.500000;
	assert( 2.500000  ==  1  == false);
	assert( 2.500000  ==  i1  == false);
	assert( i1  ==  2.500000  == false);
	assert( i1  ==  f2500000  == false);
}

float_operator_int_cmpEqual_IntCmpEqual();
