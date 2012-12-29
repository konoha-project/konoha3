import("Type.Float")

void float_operator_intMul_IntMul() { 
	int i1 = 1;
	float f2500000 = 2.500000;
	assert( 1  *  2.500000  - (2.500000e+00) <= FLOAT_EPSILON*10);
	assert( 1  *  i1  - (1) <= FLOAT_EPSILON*10);
	assert( i1  *  1  - (1) <= FLOAT_EPSILON*10);
	assert( i1  *  f2500000  - (2.500000e+00) <= FLOAT_EPSILON*10);
}

float_operator_intMul_IntMul();
