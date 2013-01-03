int fibo(int n) {
	if(n < 3) {
		return 1;
	}
	return fibo(n-1)+fibo(n-2);
}

System.p(fibo(10));
