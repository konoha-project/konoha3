// written by chenji

import("sugar");

Expr typeCheckInt(Stmt stmt, Expr expr, Gamma gma, int typeid) {
	return expr.setUnboxConstValue(4/*TY_int*/, 123);
}

Expr typeCheckBoolFalse(Stmt stmt, Expr expr, Gamma gma, int typeid) {
	return expr.setUnboxConstValue(3/*TY_boolean*/, 0);
}

Expr typeCheckBoolTrue(Stmt stmt, Expr expr, Gamma gma, int typeid) {
	return expr.setUnboxConstValue(3/*TY_boolean*/, 1);
}

addTypeCheck("true", typeCheckInt);
assert(true == 123);
addTypeCheck("true", typeCheckBoolFalse);
assert(true == false);
addTypeCheck("true", typeCheckBoolTrue);
assert(true == true);
