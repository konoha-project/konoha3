/*
 * The Great Computer Language Shootout
 * http://shootout.alioth.debian.org/
 *
 */

import("cstyle");
import("Type.Float");
import("MiniKonoha.NameSpace");
import("JavaScript.Math");
const N=2500000;

void partial(int n) {
	float twothirds = 2.0/3.0;

	float a1 = 0.0, a2 = 0.0, a3 = 0.0, a4 = 0.0, a5 = 0.0;
	float a6 = 0.0, a7 = 0.0, a8 = 0.0, a9 = 0.0, alt = -1.0;

	for (int k = 1; k <= n; k = k + 1) {
		float k2 = k * k, k3 = k2 * k;
		float sk = Math.sin(k), ck = Math.cos(k);
		alt = -alt;

		a1 = a1 + Math.pow(twothirds,k-1);
		a2 = a2 + 1.0/(Math.sqrt(k));
		a3 = a3 + 1.0/(k*(k+1.0));
		a4 = a4 + 1.0/(k3 * sk*sk);
		a5 = a5 + 1.0/(k3 * ck*ck);
		a6 = a6 + 1.0/k;
		a7 = a7 + 1.0/k2;
		a8 = a8 + alt/k;
		a9 = a9 + alt/(2.0*k -1.0);
	}
	System.p(""+a1/*.format("%0.9f")*/ + "\t(2/3)^k");
	System.p(""+a2/*.format("%0.9f")*/ + "\tk^-0.5");
	System.p(""+a3/*.format("%0.9f")*/ + "\t1/k(k+1)");
	System.p(""+a4/*.format("%0.9f")*/ + "\tFlint Hills");
	System.p(""+a5/*.format("%0.9f")*/ + "\tCookson Hills");
	System.p(""+a6/*.format("%0.9f")*/ + "\tHarmonic");
	System.p(""+a7/*.format("%0.9f")*/ + "\tRiemann Zeta");
	System.p(""+a8/*.format("%0.9f")*/ + "\tAlternating Harmonic");
	System.p(""+a9/*.format("%0.9f")*/ + "\tGregory");
}

void run() {
	int n = N;
	partial(n);
}

run();

