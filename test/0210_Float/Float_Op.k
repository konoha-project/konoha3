import("Type.Float")

boolean float_operator_Add() { 
	float f1000000 = 1.000000;
	float f2000000 = 2.000000;
	assert( 1.000000  +  2.000000  - (3.000000) <= FLOAT_EPSILON*10);
	assert( 1.000000  +  f1000000  - (2.000000) <= FLOAT_EPSILON*10);
	assert( f1000000  +  1.000000  - (2.000000) <= FLOAT_EPSILON*10);
	assert( f1000000  +  f2000000  - (3.000000) <= FLOAT_EPSILON*10);
}
boolean float_operator_sub() { 
	float f6000000 = 6.000000;
	float f3000000 = 3.000000;
	assert( 5.000000  -  4.000000  - (1.000000) <= FLOAT_EPSILON*10);
	assert( 5.000000  -  f6000000  - (-1.000000) <= FLOAT_EPSILON*10);
	assert( f6000000  -  5.000000  - (1.000000) <= FLOAT_EPSILON*10);
	assert( f6000000  -  f3000000  - (3.000000) <= FLOAT_EPSILON*10);
}
boolean float_operator_mul() { 
	float f6000000 = 6.000000;
	float f3000000 = 3.000000;
	assert( 5.000000  *  4.000000  - (20.000000) <= FLOAT_EPSILON*10);
	assert( 5.000000  *  f6000000  - (30.000000) <= FLOAT_EPSILON*10);
	assert( f6000000  *  5.000000  - (30.000000) <= FLOAT_EPSILON*10);
	assert( f6000000  *  f3000000  - (18.000000) <= FLOAT_EPSILON*10);
}
boolean float_operator_div() { 
	float f6000000 = 6.000000;
	float f3000000 = 3.000000;
	assert( 5.000000  /  4.000000  - (1.250000) <= FLOAT_EPSILON*10);
	assert( 5.000000  /  f6000000  - (0.833333333333333333333333333333333) <= FLOAT_EPSILON*10);
	assert( f6000000  /  5.000000  - (1.200000) <= FLOAT_EPSILON*10);
	assert( f6000000  /  f3000000  - (2.000000) <= FLOAT_EPSILON*10);
}
boolean float_operator_cmpEqual() { 
	float f1000000 = 1.000000;
	float f2000000 = 2.000000;
	assert( 1.000000  ==  2.000000  == false);
	assert( 1.000000  ==  f1000000  == true);
	assert( f1000000  ==  1.000000  == true);
	assert( f1000000  ==  f2000000  == false);
}
boolean float_operator_cmpNotEqual() { 
	float f1000000 = 1.000000;
	float f2000000 = 2.000000;
	assert( 1.000000  !=  2.000000  == true);
	assert( 1.000000  !=  f1000000  == false);
	assert( f1000000  !=  1.000000  == false);
	assert( f1000000  !=  f2000000  == true);
}
boolean float_operator_cmpLessThan() { 
	float f1000000 = 1.000000;
	float f2000000 = 2.000000;
	assert( 1.000000  <  2.000000  == true);
	assert( 1.000000  <  f1000000  == false);
	assert( f1000000  <  1.000000  == false);
	assert( f1000000  <  f2000000  == true);
}
boolean float_operator_cmpLessThanEqual() { 
	float f1000000 = 1.000000;
	float f2000000 = 2.000000;
	assert( 1.000000  <=  2.000000  == true);
	assert( 1.000000  <=  f1000000  == true);
	assert( f1000000  <=  1.000000  == true);
	assert( f1000000  <=  f2000000  == true);
}
boolean float_operator_cmpMoreThan() { 
	float f1000000 = 1.000000;
	float f2000000 = 2.000000;
	assert( 1.000000  >  2.000000  == false);
	assert( 1.000000  >  f1000000  == false);
	assert( f1000000  >  1.000000  == false);
	assert( f1000000  >  f2000000  == false);
}
boolean float_operator_cmpMoreThanEqual() { 
	float f1000000 = 1.000000;
	float f2000000 = 2.000000;
	assert( 1.000000  >=  2.000000  == false);
	assert( 1.000000  >=  f1000000  == true);
	assert( f1000000  >=  1.000000  == true);
	assert( f1000000  >=  f2000000  == false);
}
float_operator_Add();
float_operator_sub();
float_operator_mul();
float_operator_div();
float_operator_cmpEqual();
float_operator_cmpNotEqual();
float_operator_cmpLessThan();
float_operator_cmpLessThanEqual();
float_operator_cmpMoreThan();
float_operator_cmpMoreThanEqual();
