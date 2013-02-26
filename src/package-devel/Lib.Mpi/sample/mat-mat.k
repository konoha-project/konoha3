import("konoha");
import("Type.Bytes");
import("Type.Float");
import("Lib.Mpi");

float fabs(float f) {
	if(f < 0.0) return 0.0 - f;
	else return f;
}

void myMatMat(MPIData c, MPIData a, MPIData b, int n) {
	MPIComm w = MPIComm.getWorld();
	int myid = w.getRank();
	int numprocs = w.getSize();
	int np = n / numprocs;
	MPIData b_t = MPIData.newFloatArray(n * np);

	int dest = myid - 1;
	if(myid == 0) dest = numprocs - 1;
	int src  = myid + 1;
	if(myid == numprocs-1) src = 0;

	int l=0;
	while(l < numprocs) {
		int jstart = np * ((myid + l) % numprocs);
		int i=0;
		while(i < np) {
			int j=0;
			while(j < np) {
				int cidx = i * n + (jstart + j);
				int k=0;
				while(k < n) {
					c.setf(cidx, c.getf(cidx) + a.getf(i*n+k) * b.getf(k*np+j));
					k=k+1;
				}
				j=j+1;
			}
			i=i+1;
		}
		if(l != numprocs-1) {
			if(myid%2 == 0) {
				w.send(b, np*n, dest, l);
				w.recv(b_t, np*n, src, l+numprocs);
			} else {
				w.recv(b_t, np*n, src, l);
				w.send(b, np*n, dest, l+numprocs);
			}
			/* copy B_T to B */
			int i=0;
			while(i < n) {
				int j=0;
				while(j < np) {
					int bidx = i * np + j;
					b.setf(bidx, b_t.getf(bidx));
					j=j+1;
				}
				i=i+1;
			}
		}
		l=l+1;
	}
}

void main() {
	MPIComm w = MPIComm.getWorld();
	int nn = 480;
	int myid = w.getRank();
	int numprocs = w.getSize();
	int np = nn / numprocs;
	MPIData a = MPIData.newFloatArray(np * nn);
	MPIData b = MPIData.newFloatArray(np * nn);
	MPIData c = MPIData.newFloatArray(np * nn);

	/* ---------- matrix generation ---------- */
	int j = 0;
	while(j < np) {
		int i = 0;
		while(i < nn) {
			a.setf(j + i * np, 1.0);
			b.setf(i + j * nn, 1.0);
			c.setf(j + i * np, 0.0);
			i=i+1;
		}
		j=j+1;
	}

	/* ---------- start of mat-vec routine ---------- */
	w.barrier();
	float t1 = MPI.getWtime();

	myMatMat(c, a, b, nn);

	w.barrier();
	float t2 = MPI.getWtime();
	float t0 = t2 - t1;

	MPIData r1 = MPIData.newFloatArray(1);
	MPIData r2 = MPIData.newFloatArray(1);
	r1.setf(0, t0);
	w.reduce(r1, r2, 1, MAX, 0);
	/* ---------- end of mat-vec routine ---------- */

	if(myid == 0) {
		float tw = r2.getf(0);
		float mflops = 2.0 * nn * nn * nn / tw;
		mflops = mflops * 1.0e-6;
		System.p("N = " + nn);
		System.p("Mat-Mat time = " + tw + " [sec.]");
		System.p(" " + mflops + " [MFLOPS]");
		System.p("ok");
	}

	/* ---------- Verification ---------- */
	if(myid == 0) {
		float eps = 1.0e-18;
		j=0;
		while(j<np) {
			int i=0;
			while(i < nn) {
				if(fabs(c.getf(j + i * np) - nn) > eps) {
					System.p("error in " + j + ", " + i);
					return;
				}
				i=i+1;
			}
			j=j+1;
		}
	}
}

main();

