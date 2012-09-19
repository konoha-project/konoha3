import("konoha");
import("curl");

void test() {
	Curl c = new Curl();
	String str = "";
	c.setOpt(CURLOPT_URL, "http://www.google.com");
	c.setOpt(CURLOPT_WRITEDATA, str);
	c.perform();
	System.p(str);
}

test();
