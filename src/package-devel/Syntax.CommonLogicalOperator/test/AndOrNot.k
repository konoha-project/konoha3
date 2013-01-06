import("syntax.common.logic");

void f() {
	assert((not true) == false);
	assert((not false) == true);
	assert((not not true) == true);
	assert((true or false) == true);
	assert((true or false) == true);
	assert((true or true) == true);
	assert((false or false) == false);
	assert((false and false) == false);
	assert((false and true) == false);
	assert((true and false) == false);
	assert((true and true) == true);
	assert((not (true and false)) == (false or true));
}

f();
