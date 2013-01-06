/* written by kimio? */

import("Syntax.CStyleWhile");
import("Type.Float");

float sqrt(float y) {
	float x = y / 2;
	int i = 0;
	while (i < 10000) {
		x = (x + y/x) / 2;
		i = i + 1;
	}
	return x;
}

assert(1.4142135623730951 - sqrt(2.0) <= FLOAT_EPSILON);
