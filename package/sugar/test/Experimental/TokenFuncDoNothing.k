// written by kimio kuramitsu

K.import("sugar");

int TokenFunc(Token tk, String source) {
	System.p("source="+source);
	return 0;
}

K.addTokenizeFunc("'", TokenFunc);

''

