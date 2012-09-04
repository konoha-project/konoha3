// written by okamoto
// reviewed by nakata

K.import("konoha.global");
K.import("konoha.assign");
K.import("konoha.class");
K.import("konoha.string");
K.import("konoha.bytes");
K.import("jansson");
K.import("posix.file");
K.import("konoha.subproc");

class DSERequest {
	int taskid;
	String scripttype;
	int contextid;
	String execmethod;
	String logpool; // maybe removed in future
	String script;

	DSERequest(String msg) {
		Json jdata = Json.parse(msg);
		this.taskid = jdata.getInt("taskid");
		this.scripttype = jdata.getString("scripttype");
		this.contextid = jdata.getInt("contextid");
		this.execmethod = jdata.getString("execmethod");
		this.logpool = jdata.getString("logpool");
		this.script = jdata.getString("script");
	}
}

void main() {
	if(DSE_DEBUG == 1) {
		System.p(DSE_MESSAGE);
	}
	DSERequest req = new DSERequest(DSE_MESSAGE);
	String script = req.script;
	String scriptPath = "./" + req.contextid + ".k";
	String command = req.scripttype + " " + scriptPath;
	if(DSE_DEBUG == 1) {
		System.p(command);
	}
	FILE f = System.fopen(scriptPath, "w");
	int len = script.getSize();
	f.write(script.asBytes(), 0, len);
	f.close();
	Subproc sp = new Subproc("", false);
	sp.enableShellmode(false);
	String str = sp.exec(command);
	System.p(str);
}

main();
