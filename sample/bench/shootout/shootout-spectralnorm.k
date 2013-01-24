/*
 * The Computer Language Benchmarks Game
 * http://shootout.alioth.debian.org/
 *
 */

import("MiniKonoha.NameSpace");
import("Syntax.JavaStyleClass");
import("Syntax.JavaNewArray");
import("Syntax.CStyleFor");
import("Syntax.CStyleCompoundAssignment");
import("JavaScript.Array");
import("Type.Float");
import("JavaScript.Math");

const N=5500;

class Spectralnorm {
	Spectralnorm() {}
	/* return element i,j of infinite matrix A */
	float a(int i, int j){
		return 1.0/((i+j)*(i+j+1)/2 +i+1);
	}

	/* multiply vector v by matrix A */
	void multiplyAv(int n, float[] v, float[] av){
		for (int i=0; i < n; i= i + 1) {
			av[i] = 0;
			for (int j=0; j < n; j= j + 1) {
				av[i] = av[i] + a(i,j)*v[j];
			}
		}
	}

	/* multiply vector v by matrix A transposed */
	void multiplyAtv(int n, float[] v, float[] atv){
		for (int i=0; i < n; i= i + 1) {
			atv[i] = 0;
			for (int j=0; j < n; j= j + 1) {
				atv[i] = atv[i] + a(j,i)*v[j];
			}
		}
	}

	/* multiply vector v by matrix A and then by matrix A transposed */
	void multiplyAtAv(int n, float[] v, float[] atAv){
		float[] u = new float[n];
		multiplyAv(n, v, u);
		multiplyAtv(n, u, atAv);
	}

	float approximate(int n) {
		// create unit vector
		float[] u = new float[n];
		for (int i=0; i < n; i= i + 1) {
			u[i] = 1;
		}

		// 20 steps of the power method
		float[] v = new float[n];
		for (int i=0; i < n; i= i + 1) {
			v[i]=0;
		}

		for (int i=0; i < 10; i= i + 1) {
			multiplyAtAv(n, u, v);
			multiplyAtAv(n, v, u);
		}

		// B=AtA		 A multiplied by A transposed
		// v.Bv /(v.v)   eigenvalue of v
		float vBv = 0, vv = 0;
		for (int i=0; i < n; i= i + 1) {
			vBv = vBv + u[i]*v[i];
			vv = vv + v[i]*v[i];
		}
		return Math.sqrt(vBv/vv);
	}
}

void run() {
	int n = N;
	Spectralnorm s = new Spectralnorm();
	float res = s.approximate(n);
	System.p("" + res);
}

run();

