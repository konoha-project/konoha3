import("jansson");
import("curl");
import("konoha");

class Task {
	int taskid;
	String scripttype;
	int contextid;
	String execmethod;
	String logpool;
	String script;

	Task(int taskid, String scripttype, int contextid, String execmethod, String logpool, String script) {
		this.taskid = taskid;
		this.scripttype = scripttype;
		this.contextid = contextid;
		this.execmethod = execmethod;
		this.logpool = logpool;
		this.script = script;
	}

	void sendTo(String host, int port) {
		Json sdata = new Json();
		sdata.setInt("taskid", this.taskid);
		sdata.setString("scripttype", this.scripttype);
		sdata.setInt("contextid", this.contextid);
		sdata.setString("execmethod", this.execmethod);
		sdata.setString("logpool", this.logpool);
		sdata.setString("script", this.script);

		String req = sdata.dump();
		p(req);
		String res = "";

		Curl c = new Curl();
		String url = "http://" + host + ":" + port;
		c.setOpt(CURLOPT_URL, url);
		c.appendHeader("Content-Type: application/json");
		c.setOpt(CURLOPT_POSTFIELDS, req);
		c.setOpt(CURLOPT_WRITEDATA, res);
		c.perform();
		return;
	}
}

void main() {
	String host = "localhost";
	int port = 8080;
	String logpoolip = "0.0.0.0";
	Task t = new Task(1, "minikonoha", 1, "eval", logpoolip, "p(1);");
	t.sendTo(host, port);
	return;
}

main();
