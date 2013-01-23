/*
 * The Great Computer Language Shootout
 * http://shootout.alioth.debian.org/
 *
 */

import("cstyle");
import("JavaScript.Array");
import("Syntax.JavaStyleClass");
import("Syntax.JavaNewArray");
import("MiniKonoha.NameSpace");
import("Java.String");

const N=8;

int nsieve(int m, boolean[] isPrime) {
	int i=2;
	while( i <= m) {
		isPrime[i] = true;
		i = i + 1;
	}
	int count = 0;

	int i=2; while( i <= m) { 
		if (isPrime[i]) {
			int k=i+i; while( k <= m) { 
				isPrime[k] = false;
				k = k + i;
			}
			count = count + 1;
		}
		i = i + 1;
	}
	return count;
}

String padNumber(int number, int fieldLen) {
	String s = "" + number;
	int numSpaces = fieldLen - s.length();

	int i = 0; while( i < numSpaces) {
		s = " " + s;
		i = i + 1;
	}
	return s;
}

void run() {
	int n = N;
	int m = (1<<n)*10000;
	boolean[] flags = new boolean[m+1];

	System.p("Primes up to " + padNumber(m, 8) + " " + padNumber(nsieve(m,flags), 8));
	m = (1<<(n-1))*10000;
	System.p("Primes up to " + padNumber(m, 8) + " " + padNumber(nsieve(m,flags), 8));
	m = (1<<(n-2))*10000;
	System.p("Primes up to " + padNumber(m, 8) + " " + padNumber(nsieve(m,flags), 8));
}

run();

