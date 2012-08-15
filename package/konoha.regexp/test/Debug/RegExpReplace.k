K.import("konoha.regexp");


System.p("hoge1234".replace(/123/, "xxx"));
System.p("hoge1234".replace(/\d/, "xxx"));
System.p("hoge1234".replace(/x/, "xxx"));
System.p("hoge/1234".replace(/\//, "xxx"));
System.p("hoge1234".replace(/1.*/, "xxx"));

