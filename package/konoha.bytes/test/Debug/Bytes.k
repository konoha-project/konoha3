/* written by shinpei nakata */


K.import("konoha.bytes");
K.import("konoha");
/* byte */
System.p("hoge".toBytes());


System.p("hoge".toBytes().encodeTo("UTF-8").toString());
System.p("hoge".toBytes().encodeTo("SHIFT_JIS").toString());
System.p("hoge".toBytes().encodeTo("EUC-JP").toString());

System.p("hoge".toBytes().decodeFrom("UTF-8"));
System.p("hoge".toBytes().decodeFrom("SHIFT_JIS"));
System.p("hoge".toBytes().decodeFrom("EUC-JP"));

/* multi bytes (original is UTF-8) */
System.p("ほげほげ".toBytes());
System.p("ほげほげ".toBytes().toString());

System.p("ほげほげ".toBytes().encodeTo("UTF-8").toString());
System.p("ほげほげ".toBytes().encodeTo("SHIFT_JIS").toString());
System.p("ほげほげ".toBytes().encodeTo("EUC-JP").toString());

System.p("ほげほげ".toBytes().encodeTo("UTF-8").decodeFrom("UTF-8"));
System.p("ほげほげ".toBytes().encodeTo("SHIFT_JIS").decodeFrom("SHIFT_JIS"));
System.p("ほげほげ".toBytes().encodeTo("EUC-JP").decodeFrom("EUC-JP"));

