//// written by unknown
//
////import("konoha.new"); Don't use this
//import("Type.Bytes");
//import("Lib.Curl");
//
//void test() {
//	Curl c = new Curl();
//	Bytes buf = new Bytes(0);
//	/* FIXME: we need to care about no-network environment */
//	c.setOpt(CURLOPT_URL, "http://www.google.com");
//	c.setOpt(CURLOPT_WRITEDATA, buf);
//	c.perform();
//	assert(buf.getSize() >= 221);
//}
//
//test();
