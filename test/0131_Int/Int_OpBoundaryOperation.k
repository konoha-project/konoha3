/* this test is written by shinpei nakata */
import("Syntax.CStyleBitwiseOperator");
import("cstyle"); /* for INT_MAX, INT_MIN */
void int_operator_boundaryOperation_boundaryOperation() { 
	int max = INT_MAX;
	int min = INT_MIN;
	System.p(max + min);
	System.p(min + max);
	System.p(min + min);
	System.p(max - max);
	System.p(max - min);
	System.p(min - max);
	System.p(min - min);
	System.p(max / max);
	System.p(max / min);
	System.p(min / max);
	System.p(min / min);
}
int_operator_boundaryOperation_boundaryOperation();

