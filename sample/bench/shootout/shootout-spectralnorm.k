/*
 * The Computer Language Benchmarks Game
 * http://shootout.alioth.debian.org/
 *
 */

import("cstyle");
import("MiniKonoha.NameSpace");  // konoha.const is obsolete
import("Syntax.JavaStyleClass");
import("JavaScript.Array");
//import("konoha.new"); Don't use this
import("Type.Float");
import("JavaScript.Math");
const N=550;
//const N=5500;

class Spectralnorm {
	/* return element i,j of infinite matrix A */
	float a(int i, int j){
		return 1.0/((i+j)*(i+j+1)/2 +i+1);
	}

	/* multiply vector v by matrix A */
	void multiplyAv(int n, float[] v, float[] av){
		int i=0; while(i<n) { 
			av[i] = 0;
			int j=0;
			while(j<n) { 
				av[i] += a(i,j)*v[j];
				j = j + 1;
			}
			i = i + 1;
		}
	}

	/* multiply vector v by matrix A transposed */
	void multiplyAtv(int n, float[] v, float[] atv){
		int i=0; while(i<n) {
			atv[i] = 0;
			int j=0;
			while(j<n) {
				atv[i] += a(j,i)*v[j];
				j = j + 1;
			}
			i = i + 1;
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
		int i=0;
		while(i<n) {
			u[i] = 1; i = i + 1;
		}

		// 20 steps of the power method
		float[] v = new float[n];
		int i=0;
		while(i<n) {
			v[i]=0; i = i + 1;
		}

		int i=0; while( i<10) { 
			multiplyAtAv(n, u, v);
			multiplyAtAv(n, v, u);
			i = i + 1;
		}

		// B=AtA		 A multiplied by A transposed
		// v.Bv /(v.v)   eigenvalue of v
		float vBv = 0, vv = 0;
		int i=0;
		while( i<n) {
			vBv = vBv + u[i]*v[i];
			vv = vv + v[i]*v[i];
			i = i + 1;
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

