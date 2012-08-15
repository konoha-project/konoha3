// written by kimio

void f(int a) { // this is it
	/** please comment a lot !!
        **/
	if(a == 1) { // comment
		/** this will not happen 
		    but..   **/
		System.p("line=9");  //
		/* meaningless */ 
		if(a == 1) { 
		// this comment must be comment out
			System.p("line=13");
		// this isn't good number ;;)
		}
		/**
		  Again ??
		**/
	} // end of if
	/** Again ?? **/
	System.p("line=21");
}

f(1)


//
