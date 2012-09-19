// written by kimio kuramitsu

import("sugar");

int TokenFunc(Token tk, String source) {
	System.p("source="+source);
	return 0;
}

addTokenizeFunc("'", TokenFunc);

''

