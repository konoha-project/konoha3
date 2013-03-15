/*
 * simple http server
 * porting from C program sample/http-server.c
 */

import("Syntax.JavaStyleClass");
import("JavaStyle.Object");
import("Syntax.Null");
import("cstyle");
import("JavaScript.Array");
import("Lib.Libevent");

class CmdFlg {
	int flg;
	String type;

	CmdFlg(int f, String t) {
		flg = f;
		type = t;
	}
};

void dump_request_cb(evhttp_request req, Object arg) {
	System.p("dump_request_cb() executed!!");
	System.p("req = \n" + req);
	System.p("arg = " + arg);

	String cmdtype = "unknown";
	CmdFlg[] cmdtbl = [ 
		new CmdFlg(EVHTTP_REQ_GET, "GET"),
		new CmdFlg(EVHTTP_REQ_POST, "POST"),
		new CmdFlg(EVHTTP_REQ_HEAD, "HEAD"),
		new CmdFlg(EVHTTP_REQ_PUT, "PUT"),
		new CmdFlg(EVHTTP_REQ_DELETE, "DELETE"),
		new CmdFlg(EVHTTP_REQ_OPTIONS, "OPTIONS"),
		new CmdFlg(EVHTTP_REQ_TRACE, "TRACE"),
		new CmdFlg(EVHTTP_REQ_CONNECT, "CONNECT"),
		new CmdFlg(EVHTTP_REQ_PATCH, "PATCH")
	];

	int cmd = req.get_command();
	for (int i = 0; i < cmdtbl.getlength(); i++) {
		if ((cmdtbl[i].flg & cmd) != 0) {
			cmdtype = cmdtbl[i].type;
			break;
		}
	}
	System.p("Received a " + cmdtype + " request for " + req.get_uri());

	req.send_reply(200, "OK", NULL);
}

void send_document_cb(evhttp_request req, Object arg) {
	System.p("send_document_cb() executed!!");
	System.p("req = \n" + req);
	System.p("arg = " + arg);

	req.send_reply(200, "OK", NULL);
}

void main() {
	event_base base = new event_base();
	evhttp http = new evhttp(base);

	int ret = http.set_cb("/dump", dump_request_cb, NULL);
	System.p("http.set_cb() return: " + ret);
	http.set_gencb(send_document_cb, "/tmp");
	System.p("http.set_gencb() returned");
	evhttp_bound_socket handle = http.bind_socket_with_handle("0.0.0.0", 10000);


	base.event_dispatch();
}

main();
