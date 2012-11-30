// written by okamoto
// reviewed by nakata

//import("konoha.new");
//import("konoha.class");
//import("konoha.array");
//import("konoha.string");
//import("konoha.bytes");
//import("konoha.json");
//import("konoha.file");
//import("konoha.date");
//import("dscript.subproc");

System.p("hoge");

//class Logger {
//	Json data;
//
//	Logger() {
//		this.data = new Json();
//	}
//
//	void set(String key, String value) {
//		this.data.setString(key, value);
//	}
//
//	void out() {
//		String str = this.data.toString();
//		if(DSE_DEBUG == 1) {
//			System.p("log:" + str);
//		}
//		String command = "logger -t minikonoha " + str.replace("\"", "\\" + "\"").replace("(", "\\" + "(").replace(")", "\\" + ")");
//		System.system(command);
//	}
//}

void executeScript() {
//	System.p(DSE_MESSAGE);
//	Json message = Json.parse(DSE_MESSAGE);
//	String cid = e.getProperty("CId");
//	String tid;
//	String name;
//	if(e.getProperty("event") == "D-Control") {
//		name = cid;
//		tid = "0";
//	}
//	else if(e.getProperty("event") == "D-Task") {
//		name = e.getProperty("Name");
//		tid = e.getProperty("TId");
//	}
//	else {
//		System.p("DSE handles only D-Script");
//		return;
//	}
//	String scriptname = cid + ":" + tid + ".k"
//	String script = e.getProperty("Script");
//	String option = e.getProperty("Option");
//	String To = e.getProperty("To");
//	String From = e.getProperty("From");
//	String[] strs = To.split(":");
//	String localhost = "\\" + "\"" + strs[0] + "\\" + "\"";
//	int    port = strs[1] to int;
//
//	FILE f = new FILE("script/" + scriptname, "w");
//	f.write(script to Bytes);
//	f.close();
//	System.p("dse received script"); // for debug
//
//	String command = "minikonoha"
//	System.p("command:" + command); // for debug
//	String[] arglist = ("-DUSE_PRINT=1 -DHOST=" + localhost + " -DPORT=" + port + " -MSyslog -MDScriptConsole -Ikonoha.console -Idscript.shell script/" + scriptname).split(" ");
//	if(option == "Test") {
//		arglist.add("-MTraceVM");
//	}
//	System.p("arglist:" + arglist); // for debug
//	SubProc sp = new SubProc(command);
//	sp.setArgumentList(arglist);
//
//	Logger startlog = new Logger();
//	startlog.set("Method", "StartTask");
//	startlog.set("CId", cid);
//	startlog.set("TId", tid);
//	startlog.set("State", "start");
//	startlog.set("ScriptName", scriptname);
//	startlog.set("Ip", To);
//	Date starttime = new Date();
//	startlog.set("Time", starttime.getTime() to String);
//	startlog.out();
//
//	int result = sp.fg();
//	System.p("result:" + result); // for debug
//
//	Logger endlog = new Logger();
//	endlog.set("Method", "EndTask");
//	endlog.set("State", "end");
//	endlog.set("CId", cid);
//	endlog.set("TId", tid);
//	Json sdata = new Json();
//	sdata.setString("event", tid);
//
//	if(result == DTASK_SUCCESS) {
//		endlog.set("Result", "success");
//		sdata.setString("Result", "success");
//	}
//	else {
//		endlog.set("Result", "error");
//		sdata.setString("result", "error");
//	}
//
//	endlog.set("ScriptName", scriptname);
//	endlog.set("Ip", To);
//	Date endtime = new Date();
//	endlog.set("Time", endtime.getTime() to String);
//	endlog.out();
//	System.p("dse executed script"); // for debug
//
//	String response = sdata.dump();
//	System.p("response:" + response); // for debug
//	Curl c = new Curl();
//	String url = "http://" + From;
//	c.setOpt(CURLOPT_URL, url);
//	c.setOpt(CURLOPT_POSTFIELDS, response);
//	c.perform();
//	System.p("dse sended response");
//
//	System.p(error);
}

//class DSERequest {
//	int taskid;
//	String scripttype;
//	int contextid;
//	String execmethod;
//	String logpool; // maybe removed in future
//	String script;
//
//	DSERequest(String msg) {
//		Json jdata = Json.parse(msg);
//		this.taskid = jdata.getInt("taskid");
//		this.scripttype = jdata.getString("scripttype");
//		this.contextid = jdata.getInt("contextid");
//		this.execmethod = jdata.getString("execmethod");
//		this.logpool = jdata.getString("logpool");
//		this.script = jdata.getString("script");
//	}
//}
//
//void main() {
//	if(DSE_DEBUG == 1) {
//		System.p(DSE_MESSAGE);
//	}
//	DSERequest req = new DSERequest(DSE_MESSAGE);
//	String script = req.script;
//	String scriptPath = DSE_SCRIPT_DIR + "/" + req.contextid + ".k";
//	String command = req.scripttype + " " + scriptPath;
//	if(DSE_DEBUG == 1) {
//		System.p(command);
//	}
//	FILE f = System.fopen(scriptPath, "w");
//	int len = script.getSize();
//	f.write(script.asBytes(), 0, len);
//	f.close();
//	Subproc sp = new Subproc("", false);
//	sp.enableShellmode(false);
//	String str = sp.exec(command);
//	System.p(str);
//}

//executeScript();
