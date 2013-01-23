/*
 * The Computer Language Benchmarks Game
 * http://shootout.alioth.debian.org/
 *
 */

import("JavaScript.Array");
import("MiniKonoha.NameSpace");
import("Syntax.JavaStyleClass");
import("Syntax.JavaNewArray");
import("Syntax.CStyleFor");
import("Syntax.CStyleWhile");

const N=9;
int max(int a, int b) {
    if (a > b) {
        return a;
    }
    else {
        return b;
    }
}

int fannkuch(int n) {
    int[] perm  = new int[n];
    int[] perm1 = new int[n];
    int[] count = new int[n];
    int maxFlipsCount = 0;
    int permCount = 0;
    int checksum = 0;

    for (int i=0; i < n; i = i + 1) {
        perm1[i] = i;
    }
    int r = n;

    while (true) {
        while (r != 1) { count[r-1] = r; r = r - 1; }

        for (int i=0; i < n; i = i + 1) {
            perm[i] = perm1[i];
        }

        int flipsCount = 0;
        int k;

        while ( !((k=perm[0]) == 0) ) {
            int k2 = (k+1) >> 1;

            for (int i=0; i < k2; i = i + 1) {
                int temp = perm[i]; perm[i] = perm[k-i]; perm[k-i] = temp;
            }
            flipsCount = flipsCount + 1;
        }

        maxFlipsCount = max(maxFlipsCount, flipsCount);
        if(permCount%2 == 0) {
            checksum = checksum + flipsCount;
        } else {
            checksum = checksum - flipsCount;
        }

        // Use incremental change to generate another permutation
        while (true) {
            if (r == n) {
                System.p(""+checksum);
                return maxFlipsCount;
            }
            int perm0 = perm1[0];

            for (int i=0; i < r; i = i + 1) {
                perm1[i] = perm1[i+1];
            }
            perm1[r] = perm0;

            count[r] = count[r] - 1;
            if (count[r] > 0) break;
            r = r + 1;
        }

        permCount = permCount + 1;
    }
    return 0;
}

void run() {
    int n = N;
    System.p("Pfannkuchen("+n+") = "+fannkuch(n));
}

run();
