// written by okamoto
// reviewed by nakata

import("jansson"); // Replace to konoha.json
import("konoha.class");
import("konoha.array");
import("konoha.const");
import("konoha.string");
import("konoha.bytes");
import("konoha.file");
import("konoha.date");
import("posix.process");
import("dscript.subproc");

const DTASK_SUCCESS = 0;

class Logger {
	Json data;

	Logger() {
		this.data = new Json();
	}

	void set(String key, String value) {
		this.data.setString(key, value);
	}

	void out() {
		String str = this.data.toString();
		if(DSE_DEBUG == 1) {
			System.p("log:" + str); // for debug
		}
		String command = "logger -t minikonoha " + str.replace("\"", "\\" + "\"").replace("(", "\\" + "(").replace(")", "\\" + ")");
		System.system(command);
	}
}

void executeScript() {
	Json message = Json.parse(DSE_MESSAGE);
	String cid = message.getString("CId");
	String tid;
	String name;
	if(message.getString("event") == "D-Control") {
		name = cid;
		tid = "0";
	}
	else if(message.getString("event") == "D-Task") {
		name = message.getString("Name");
		tid = message.getString("TId");
	}
	else {
		System.p("DSE handles only D-Script");
		return;
	}
	String scriptname = cid + ":" + tid + ".k"
	String script = message.getString("Script");
	String option = message.getString("Option");
	String To = message.getString("To");
	String From = message.getString("From");
	String[] strs = To.split(":");
	String localhost = "\\" + "\"" + strs[0] + "\\" + "\"";
	int    port = strs[1].toint();

	FILE f = new FILE(scriptname, "w");
	f.write(script.toBytes());
	f.close();
	if(DSE_DEBUG == 1) {
		System.p("dse received script"); // for debug
	}

	String command = "minikonoha"
	if(DSE_DEBUG == 1) {
		System.p("command:" + command); // for debug
	}
	String[] arglist = ("-DUSE_PRINT=1 -DHOST=" + localhost + " -DPORT=" + port + " -MSyslog -MDScriptConsole -Ikonoha.console -Idscript.shell " + scriptname).split(" ");
	if(option == "Test") {
		arglist.add("-MTraceVM");
	}
	if(DSE_DEBUG == 1) {
		System.p("arglist:" + arglist); // for debug
	}
	SubProc sp = new SubProc(command);
	sp.setArgumentList(arglist);

	Logger startlog = new Logger();
	startlog.set("Method", "StartTask");
	startlog.set("CId", cid);
	startlog.set("TId", tid);
	startlog.set("State", "start");
	startlog.set("ScriptName", scriptname);
	startlog.set("Ip", To);
	Date starttime = new Date();
	startlog.set("Time", starttime.getTime().toString());
	startlog.out();

	int result = sp.fg();
	if(DSE_DEBUG == 1) {
		System.p("result:" + result); // for debug
	}

	Logger endlog = new Logger();
	endlog.set("Method", "EndTask");
	endlog.set("State", "end");
	endlog.set("CId", cid);
	endlog.set("TId", tid);

	if(result == DTASK_SUCCESS) {
		endlog.set("Result", "success");
	}
	else {
		endlog.set("Result", "error");
	}

	endlog.set("ScriptName", scriptname);
	endlog.set("Ip", To);
	Date endtime = new Date();
	endlog.set("Time", endtime.getTime().toString());
	endlog.out();
	if(DSE_DEBUG == 1) {
		System.p("dse executed script"); // for debug
	}
}

executeScript();
