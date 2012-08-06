K.import("konoha.pcre");


System.p("hoge1234".search(/123/));
System.p("hoge1234".search(/\d/));
System.p("hoge1234".search(/x/));
System.p("hoge/1234".search(/\//));
System.p("hoge1234".search(/1.*/));

