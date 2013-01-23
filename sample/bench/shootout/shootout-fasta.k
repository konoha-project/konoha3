/*
 * The Computer Language Benchmarks Game
 * http://shootout.alioth.debian.org/
 *
 */

import("cstyle");
import("MiniKonoha.NameSpace");
import("Syntax.GlobalVariable");
import("Syntax.JavaStyleClass");
import("JavaScript.Array");
import("Type.Float");
import("Type.Bytes");
import("Type.File");

AllowImplicitGlobalVariable(true);

const N=250000;
const EOL = "\n";

const ALU = ("GGCCGGGCGCGGTGGCTCACGCCTGTAATCCCAGCACTTTGG" + 
 "GAGGCCGAGGCGGGCGGATCACCTGAGGTCAGGAGTTCGAGA" +
 "CCAGCCTGGCCAACATGGTGAAACCCCGTCTCTACTAAAAAT" +
 "ACAAAAATTAGCCGGGCGTGGTGGCGCGCGCCTGTAATCCCA" +
 "GCTACTCGGGAGGCTGAGGCAGGAGAATCGCTTGAACCCGGG" +
 "AGGCGGAGGTTGCAGTGAGCCGAGATCGCGCCACTGCACTCC" +
 "AGCCTGGGCGACAGAGCGAGACTCCGTCTCAAAAA");

class Frequency {
	int c;
	float p;
	Frequency(int c, float p) {
		this.c = c;
		this.p = p;
	}
}

Frequency[] iub = [new Frequency('a', 0.27), new Frequency('c', 0.12), new Frequency('g', 0.12), new Frequency('t', 0.27), new Frequency('B', 0.02), new Frequency('D', 0.02), new Frequency('H', 0.02), new Frequency('K', 0.02), new Frequency('M', 0.02), new Frequency('N', 0.02), new Frequency('R', 0.02), new Frequency('S', 0.02), new Frequency('V', 0.02), new Frequency('W', 0.02), new Frequency('Y', 0.02)];
Frequency[] homosapiens = [ new Frequency('a', 0.3029549426680), new Frequency('c', 0.1979883004921), new Frequency('g', 0.1975473066391), new Frequency('t', 0.3015094502008)];

const IM=139968;
const IA=3877;
const IC=29573;

const LINE_LENGTH=60;
const BUFFER_SIZE=1024;

// pseudo-random number generator
int last = 42;
float random(float max) {
	last = (last * IA + IC) % IM;
	return max * last / IM;
}

Bytes alub = ALU.toBytes();

void makeCumulative(Frequency[] a) {
	float cp = 0.0;
	int i = 0; while(i < a.size) {
		cp = cp + a[i].p;
		a[i].p = cp;
		i = i + 1;
	}
}

// naive
int selectRandom(Frequency[] a) {
	int len = a.size;
	float r = random(1.0);
	int i = 0; while(i < len) {
		if (r < a[i].p) {
			return a[i].c;
		}
		i = i + 1;
	}
	return a[len - 1].c;
}

int index = 0;
Bytes bbuffer = new Bytes(BUFFER_SIZE);
void makeRandomFasta(String id, String desc,Frequency[] a, int n, FILE writer) {
	index = 0;
	int m = 0;
	String descStr = ">" + id + " " + desc + EOL;
	writer.print(descStr);
	while (n > 0) {
		if (n < LINE_LENGTH) { m = n; } else { m = LINE_LENGTH; }
		if(BUFFER_SIZE - index < m){
			writer.write(bbuffer, 0, index);
			index = 0;
		}
		int i = 0; while(i < m) { i = i + 1;
			bbuffer[index] = selectRandom(a);
			index = index + 1;
		}
		bbuffer[index] = "\n".toBytes()[0];
		index = index + 1;
		n = n - LINE_LENGTH;
	}
	if(index != 0) writer.write(bbuffer, 0, index);
}

void makeRepeatFasta(String id, String desc, String alu, int n, FILE writer) {
	index = 0;
	int m = 0;
	int k = 0;
	int kn = alub.size;
	String descStr = ">" + id + " " + desc + EOL;
	writer.print(descStr);
	while (n > 0) {
		if (n < LINE_LENGTH) { m = n; } else { m = LINE_LENGTH; }
		if(BUFFER_SIZE - index < m) {
			writer.write(bbuffer, 0, index);
			index = 0;
		}
		int i = 0; while(i < m) { i = i + 1;
			if (k == kn) { k = 0; }
			bbuffer[index] = alub[k];
			index = index + 1;
			k = k + 1;
		}
		bbuffer[index] = "\n".toBytes()[0];
		index = index + 1;
		n = n - LINE_LENGTH;
	}
	if(index != 0) writer.write(bbuffer, 0, index);
}

void run() {
	int n = N;
	makeCumulative(homosapiens);
	makeCumulative(iub);
	FILE out = new FILE("tmp.fasta", "w");
	makeRepeatFasta("ONE", "Homo sapiens alu", ALU, n * 2, out);
	makeRandomFasta("TWO", "IUB ambiguity codes", iub, n * 3, out);
	makeRandomFasta("THREE", "Homo sapiens frequency", homosapiens, n * 5, out);
	out.close();
}

run();

