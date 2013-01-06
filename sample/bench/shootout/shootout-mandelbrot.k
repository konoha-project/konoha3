/*
 * The Computer Language Benchmarks Game
 * http://shootout.alioth.debian.org/
 *
 */

import("Syntax.JavaStyleClass");
import("Type.File");
import("MiniKonoha.NameSpace");
import("Type.Bytes");
import("Type.Float");
import("cstyle");

const BUFFER_SIZE = 8192;

class Mandelbrot {
    int size;
    FILE ous;
    Bytes buf;
    int bufLen = 0;
    float fac;
    int shift;

    Mandelbrot(int size) {
        this.size = size;
        fac = 2.0 / size;
        ous = new FILE("Mandelbrot.ppm", "w");
        if (size % 8 == 0) {
            shift = 0;
        } else {
            shift = 8 - (size % 8);
        }
        buf = new Bytes(BUFFER_SIZE);
    }
    void compute() {
        ous.println("P4");
        ous.println("" + size + " " + size);
        for (int y = 0; y<size; y = y + 1) {
            computeRow(y);
        }
        ous.write( buf, 0, bufLen);
        ous.close();
    }

    void computeRow(int y) {
        int bits = 0;

        float ci = (y*fac - 1.0);
        Bytes bufLocal = buf;
        for (int x = 0; x<size;x = x + 1) {
            float zr = 0.0;
            float zi = 0.0;
            float cr = (x*fac - 1.5);
            int i = 50;
            float zrN = 0;
            float ziN = 0;
            do {
                zi  = 2.0 * zr * zi + ci;
                zr  = zrN - ziN + cr;
                ziN = zi * zi;
                zrN = zr * zr;
                if (ziN + zrN > 4.0) {
                    break;
                }
                i = i - 1;
            } while (i > 0);

            bits = bits << 1;
            if (i == 0) bits++;

            if (x % 8 == 7) {
                bufLocal[bufLen] = bits;
                bufLen += 1;
                if ( bufLen == BUFFER_SIZE) {
                    ous.write(bufLocal, 0, BUFFER_SIZE);
                    bufLen = 0;
                }
                bits = 0;
            }
        }
        if (shift!=0) {
            bits = bits << shift;
            bufLocal[bufLen] = bits;
            bufLen += 1;
            if ( bufLen == BUFFER_SIZE) {
                ous.write(bufLocal, 0, BUFFER_SIZE);
                bufLen = 0;
            }
        }
    }
}

void run() {
    int n = 1600;
    new Mandelbrot(n).compute();
}

run();
