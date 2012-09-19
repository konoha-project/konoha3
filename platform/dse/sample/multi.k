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

String makeFibo(int i) {
	String fibo = "import(\"konoha\"); int fibo(int i) { if(i < 3) return i; else return fibo(i - 1) + fibo(i - 2); }; p(fibo(" + i + "));";
	return fibo;
}

void main() {
	String host = "localhost";
	int port = 8080;
	String logpoolip = "0.0.0.0";
	Task t1 = new Task(1, "minikonoha", 1, "eval", logpoolip, makeFibo(31));
	Task t2 = new Task(2, "minikonoha", 2, "eval", logpoolip, makeFibo(32));
	Task t3 = new Task(3, "minikonoha", 3, "eval", logpoolip, makeFibo(33));
	Task t4 = new Task(4, "minikonoha", 4, "eval", logpoolip, makeFibo(34));
	Task t5 = new Task(5, "minikonoha", 5, "eval", logpoolip, makeFibo(35));
	Task t6 = new Task(6, "minikonoha", 6, "eval", logpoolip, makeFibo(36));
	Task t7 = new Task(7, "minikonoha", 7, "eval", logpoolip, makeFibo(37));
	Task t8 = new Task(8, "minikonoha", 8, "eval", logpoolip, makeFibo(38));
	t1.sendTo(host, port);
	t2.sendTo(host, port);
	t3.sendTo(host, port);
	t4.sendTo(host, port);
	t5.sendTo(host, port);
	t6.sendTo(host, port);
	t7.sendTo(host, port);
	t8.sendTo(host, port);
	return;
}

main();
