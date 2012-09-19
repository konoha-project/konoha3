
void global_setup() { 
	import("konoha.float")
}

global_setup();
boolean float_operator_add() { 
	float f1000000 = 1.000000;
	float f2000000 = 2.000000;
	System.p( 1.000000  +  2.000000 );
	System.p( 1.000000  +  f1000000 );
	System.p( f1000000  +  1.000000 );
	System.p( f1000000  +  f2000000 );
}
boolean float_operator_sub() { 
	float f6000000 = 6.000000;
	float f3000000 = 3.000000;
	System.p( 5.000000  -  4.000000 );
	System.p( 5.000000  -  f6000000 );
	System.p( f6000000  -  5.000000 );
	System.p( f6000000  -  f3000000 );
}
boolean float_operator_mul() { 
	float f6000000 = 6.000000;
	float f3000000 = 3.000000;
	System.p( 5.000000  *  4.000000 );
	System.p( 5.000000  *  f6000000 );
	System.p( f6000000  *  5.000000 );
	System.p( f6000000  *  f3000000 );
}
boolean float_operator_div() { 
	float f6000000 = 6.000000;
	float f3000000 = 3.000000;
	System.p( 5.000000  /  4.000000 );
	System.p( 5.000000  /  f6000000 );
	System.p( f6000000  /  5.000000 );
	System.p( f6000000  /  f3000000 );
}
boolean float_operator_cmpEqual() { 
	float f1000000 = 1.000000;
	float f2000000 = 2.000000;
	System.p( 1.000000  ==  2.000000 );
	System.p( 1.000000  ==  f1000000 );
	System.p( f1000000  ==  1.000000 );
	System.p( f1000000  ==  f2000000 );
}
boolean float_operator_cmpNotEqual() { 
	float f1000000 = 1.000000;
	float f2000000 = 2.000000;
	System.p( 1.000000  !=  2.000000 );
	System.p( 1.000000  !=  f1000000 );
	System.p( f1000000  !=  1.000000 );
	System.p( f1000000  !=  f2000000 );
}
boolean float_operator_cmpLessThan() { 
	float f1000000 = 1.000000;
	float f2000000 = 2.000000;
	System.p( 1.000000  <  2.000000 );
	System.p( 1.000000  <  f1000000 );
	System.p( f1000000  <  1.000000 );
	System.p( f1000000  <  f2000000 );
}
boolean float_operator_cmpLessThanEqual() { 
	float f1000000 = 1.000000;
	float f2000000 = 2.000000;
	System.p( 1.000000  <=  2.000000 );
	System.p( 1.000000  <=  f1000000 );
	System.p( f1000000  <=  1.000000 );
	System.p( f1000000  <=  f2000000 );
}
boolean float_operator_cmpMoreThan() { 
	float f1000000 = 1.000000;
	float f2000000 = 2.000000;
	System.p( 1.000000  >  2.000000 );
	System.p( 1.000000  >  f1000000 );
	System.p( f1000000  >  1.000000 );
	System.p( f1000000  >  f2000000 );
}
boolean float_operator_cmpMoreThanEqual() { 
	float f1000000 = 1.000000;
	float f2000000 = 2.000000;
	System.p( 1.000000  >=  2.000000 );
	System.p( 1.000000  >=  f1000000 );
	System.p( f1000000  >=  1.000000 );
	System.p( f1000000  >=  f2000000 );
}
float_operator_add();
float_operator_sub();
float_operator_mul();
float_operator_div();
float_operator_cmpEqual();
float_operator_cmpNotEqual();
float_operator_cmpLessThan();
float_operator_cmpLessThanEqual();
float_operator_cmpMoreThan();
float_operator_cmpMoreThanEqual();

