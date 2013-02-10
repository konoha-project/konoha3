import("Syntax.Null");
import("Syntax.CStyleWhile");
import("Syntax.GoStyleIncrement");
import("Syntax.GlobalVariable");
import("Type.Bytes");
import("Type.File");
import("Type.Json");
import("Java.Class");
import("JavaScript.String");
import("JavaScript.Array");
import("JavaScript.Date");
import("Lib.Curl");
import("MiniKonoha.NameSpace");
import("MiniKonoha.Map");
import("MiniKonoha.EventListener");
import("posix.process");

load("/etc/dse/config.k");

int cid = new Date().getTime();
int tid = 1;
String IP;
Json DTaskStatusMap = new Json(); // TODO replace it to Map[Json]

const running = 0;
const done    = 1

void DTaskWatcher(Event event) {
	String id = event.getProperty("cid") + "+" + event.getInt("tid");
	Json response = new Json();
	response.setInt("code", event.getInt("code"));
	response.setString("stdout", event.getProperty("stdout"));
	response.setString("stderr", event.getProperty("stderr"));
	global.DTaskStatusMap.set(id, response);
}

boolean isMasterDseAlive() {
	Json request = new Json();
	request.setString("method", "IsDseAlive");
	Curl curl = new Curl();
	curl.setOpt(CURLOPT_URL, "http://" + HOST + ":" + PORT);
	curl.setOpt(CURLOPT_POSTFIELDS, request.toString());
	return curl.perform();
}

class DControl {
	@Public @Static boolean start(int port) {
		if(isMasterDseAlive() == false) {
			return false;
		}
		global.IP = "http://" + HOST + ":" + port;
		EventHandler.setCallBack("dse", DTaskWatcher);
		HttpEventListener.start("0.0.0.0", port);
		return true;
	}
}

String readFile(String path) {
	FILE file = new FILE(path, "rb");
	int len = 0;
	String tmp = "";
	while(true) {
		int ch = file.getc();
		if(ch == -1) {
			break;
		}
		len = len + 1;
	}
	file.close();

	if (len == 0) {
		return "";
	}

	Bytes buf = new Bytes(len);
	file = new FILE(path, "r");
	file.read(buf, 0, len);
	file.close();
	return new String(buf);
}

class DTask {
	String id;
	String script;

	DTask() {
		this.id = null;
	}

	@Public DTask(String filePath) {
		this.id = null;
		this.script = readFile(filePath).replace("\"", "\\\"");
	}

	@Public @Static DTask CreateFromText(String script) {
		DTask task = new DTask();
		task.script = script.replace("\"", "\\\"");
		return task;
	}

	String execute(String to) {
		Json request = new Json();
		request.setString("method", "ExecuteScript");
		request.setString("from", global.IP);
		Json params = new Json();
		params.setString("scripttype", "D-Task");
		params.setString("script", this.script);
		params.setString("cid", "" + global.cid);
		params.setInt("tid", global.tid);
		this.id = "" + global.cid + "+" + global.tid;
		global.tid = global.tid + 1;
		request.set("params", params);
		global.DTaskStatusMap.set(this.id, new Json());
		Curl curl = new Curl();
		curl.setOpt(CURLOPT_URL, to);
		curl.setOpt(CURLOPT_POSTFIELDS, request.toString());
		curl.perform();
		return this.id;
	}
}

class Dse {
	String IP;
	DTask[] dtasks;
	String[] dtaskIds;

	@Public Dse(String host, int port) {
		this.IP = "http://" + host + ":" + port;
		this.dtasks = [];
		this.dtaskIds = [];
	}

	@Public Dse(String host) {
		this.IP = "http://" + host + ":" + 8080;
		dtasks = [];
		dtaskIds = [];
	}

	@Public void appendDTask(DTask dtask) {
		this.dtasks.add(dtask);
	}

	@Public void appendDTask(String filePath) {
		this.dtasks.add(new DTask(filePath));
	}

	Json[] waitDTasks() {
		Json[] responses = [];
		Json response = new Json();
		int i;
		int size;
		String dtaskId;
		while(true) {
			i = 0;
			while(i < this.dtaskIds.getSize()) {
				dtaskId = this.dtaskIds[i];
				response = global.DTaskStatusMap.get(dtaskId);
				if(response.toString() != "{}") {
					responses.add(response);
					this.dtaskIds.removeAt(i);
				}
				else {
					i++;
				}
			}
			if(this.dtaskIds.getSize() == 0) {
				break;
			}
			System.usleep(10000);
		}
		return responses;
	}

	@Public Json[] dispatchDTasks() {
		int i = 0;
		int size = this.dtasks.getSize();
		Json[] responses = [];
		while(i < size) {
			this.dtaskIds.add(this.dtasks[i].execute(this.IP));
			i++;
		}
		responses =  waitDTasks();
		dtasks = [];
		dtaskIds = [];
		return responses;
	}

	String[] dispatchDTasksWithNoBlocking() {
		String[] ret;
		int i = 0;
		int size = this.dtasks.getSize();
		Json[] responses = [];
		while(i < size) {
			this.dtaskIds.add(this.dtasks[i].execute(this.IP));
			i++;
		}
		ret = this.dtaskIds;
		this.dtasks = [];
		this.dtaskIds = [];
		return ret;
	}
}

class DseGroup {
	Dse[] dses;
	DTask[] dtasks;
	String[] dtaskIds;

	@Public DseGroup() {
		this.dses = [];
		this.dtasks = [];
		this.dtaskIds = [];
	}

	@Public void appendDse(String host, int port) {
		this.dses.add(new Dse(host, port));
	}

	@Public void appendDse(String host) {
		this.dses.add(new Dse(host));
	}

	@Public void appendDse(Dse dse) {
		this.dses.add(dse);
	}

	@Public void appendDTask(String filePath) {
		int i = 0;
		int size = this.dses.getSize();
		DTask dtask = new DTask(filePath);
		while(i < size) {
			this.dses[i].appendDTask(dtask);
			i++;
		}
	}

	@Public void appendDTask(DTask dtask) {
		int i = 0;
		int size = this.dses.getSize();
		while(i < size) {
			this.dses[i].appendDTask(dtask);
			i++;
		}
	}

	Json[] waitDTasks() {
		Json[] responses = [];
		Json response = new Json();
		int i;
		int size;
		String dtaskId;
		while(true) {
			i = 0;
			while(i < this.dtaskIds.getSize()) {
				dtaskId = this.dtaskIds[i];
				response = global.DTaskStatusMap.get(dtaskId);
				if(response.toString() != "{}") {
					responses.add(response);
					this.dtaskIds.removeAt(i);
				}
				else {
					i++;
				}
			}
			if(this.dtaskIds.getSize() == 0) {
				break;
			}
			System.usleep(10000);
		}
		return responses;
	}

	@Public Json[] dispatchDTasks() {
		int i = 0;
		int j = 0; // TODO remove it
		int size = this.dses.getSize();
		Json[] responses = [];
		while(i < size) {
//			this.dtaskIds.concat(this.dses[i].dispatchDTasksWithNoBlocking());  TODO use concat in this way
			String[] ids = this.dses[i].dispatchDTasksWithNoBlocking();
			while(j < ids.getSize()) {
				this.dtaskIds.add(ids[j]);
				j = j + 1;
			}
			j = 0;
			i++;
		}
		responses =  waitDTasks();
		dtasks = [];
		dtaskIds = [];
		return responses;
	}
}
