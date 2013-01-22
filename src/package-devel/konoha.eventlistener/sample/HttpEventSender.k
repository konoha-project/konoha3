import("Type.Json");
import("Lib.Curl");

void sendHttpEvent(String host, int port) {
	Json sdata = new Json();
	sdata.setString("event", "hoge");
	String req = sdata.toString();
	String res = "";
	Curl c = new Curl();
	String url = "http://" + host + ":" + port;
	c.setOpt(CURLOPT_URL, url);
//	c.appendHeader("Content-Type: application/json");
	c.setOpt(CURLOPT_POSTFIELDS, req);
	c.perform();
}

void test() {
	sendHttpEvent("0.0.0.0", 8080);
}

test();
