/* written by shinpei nakata */


K.import("konoha.bytes");
//K.import("konoha");   // don't use konoha package in test

/* byte */
System.p("hoge".asBytes());


System.p("hoge".asBytes().encodeTo("UTF-8").asString());
System.p("hoge".asBytes().encodeTo("SHIFT_JIS").asString());
System.p("hoge".asBytes().encodeTo("EUC-JP").asString());

System.p("hoge".asBytes().decodeFrom("UTF-8"));
System.p("hoge".asBytes().decodeFrom("SHIFT_JIS"));
System.p("hoge".asBytes().decodeFrom("EUC-JP"));

/* multi bytes (original is UTF-8) */
System.p("ほげほげ".asBytes());
System.p("ほげほげ".asBytes().asString());

System.p("ほげほげ".asBytes().encodeTo("UTF-8").asString());
System.p("ほげほげ".asBytes().encodeTo("SHIFT_JIS").asString());
System.p("ほげほげ".asBytes().encodeTo("EUC-JP").asString());

System.p("ほげほげ".asBytes().encodeTo("UTF-8").decodeFrom("UTF-8"));
System.p("ほげほげ".asBytes().encodeTo("SHIFT_JIS").decodeFrom("SHIFT_JIS"));
System.p("ほげほげ".asBytes().encodeTo("EUC-JP").decodeFrom("EUC-JP"));

