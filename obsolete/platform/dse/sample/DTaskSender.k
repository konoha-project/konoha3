#!/usr/local/bin/minikonoha

import("jansson");
import("curl");
import("konoha.class");
import("konoha.ns");  // konoha.const is obsolete
import("JavaScript.String");
import("konoha.bytes");
import("konoha.file");

const BUFSIZ = 4096;

void sendDTask() {
	String to = "127.0.0.1:8080";
	String from = "127.0.0.1:80";

	FILE script = new FILE(SCRIPT, "r");
	Bytes buf = new Bytes(BUFSIZ);
	buf.setAll(0);
	script.read(buf);
	script.close();

	Json sdata = new Json();
	sdata.setString("event", "D-Control");
	sdata.setString("Script", new String(buf));
	sdata.setString("CId", "cid");
	sdata.setString("To", to);
	sdata.setString("From", from);
	sdata.setString("Option", "Operate");
	String req = sdata.toString();
	Curl c = new Curl();
	c.setOpt(CURLOPT_URL, "http://" + to);
	c.setOpt(CURLOPT_POSTFIELDS, req);
	c.perform();
}

sendDTask();
