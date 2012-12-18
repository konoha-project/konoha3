// written by kimio

void test() { 
	boolean btrue = true;
	boolean bfalse = false;
	System.p( true  &&  true  );
	System.p( true  &&  false );
	System.p( true  &&  btrue );
	System.p( true  &&  bfalse);

	System.p( btrue  &&  true  );
        System.p( btrue  &&  false );
        System.p( btrue  &&  btrue );
        System.p( btrue  &&  bfalse);
        
	System.p( false  &&  true  );
        System.p( false  &&  false );
        System.p( false  &&  btrue );
        System.p( false  &&  bfalse);
        
	System.p( bfalse  &&  true  );
        System.p( bfalse  &&  false );
        System.p( bfalse  &&  btrue );
        System.p( bfalse  &&  bfalse);
        
}

test();

