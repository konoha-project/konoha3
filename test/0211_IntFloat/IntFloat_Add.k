import("Type.Float")

void float_operator_intAdd_IntAdd() { 
	int i1 = 1;
	float f2500000 = 2.500000;
	assert( 1  +  2.500000  - (3.500000) <= FLOAT_EPSILON*10);
	assert( 1  +  i1  - (2) <= FLOAT_EPSILON*10);
	assert( i1  +  1  - (2) <= FLOAT_EPSILON*10);
	assert( i1  +  f2500000  - (3.500000) <= FLOAT_EPSILON*10);
}

float_operator_intAdd_IntAdd();
