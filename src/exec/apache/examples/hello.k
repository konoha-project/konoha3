import("Syntax.CStyleWhile");
import("JavaScript.Array");
import("Lib.Apache");

@Public int System.handler(Request req) {
	req.setContentType("text/html");
	req.setContentEncoding("utf-8");
	req.logError(APLOG_WARNING, 0, "log error's test");
	req.puts("<p><strong>hello world!</strong></p>\n");
	req.puts("<p>METHOD: "+ req.getMethod() + "</p>\n");
	req.puts("<p>ARGS: " + req.getArgs() + "</p>\n");
	req.puts("<p>URI: " + req.getUri() + "</p>\n");
	req.puts("<p>PATH_INFO: " + req.getPathInfo() + "</p>\n");
	req.puts("<p>HANDLER: " + req.getHandler() + "</p>\n");
	AprTable reqHeaders = req.getHeadersIn();
	AprTable resHeaderTable = req.getHeadersOut();
	resHeaderTable.add("key1", "val1");
	resHeaderTable.add("key2", "val2");
	resHeaderTable.set("key1", "val3");
	resHeaderTable.add("key1", "val4");
	resHeaderTable.add("key2", "値5");
	AprTableEntry[] resHeaders = resHeaderTable.getElts();
	req.puts("<p>size = " + resHeaders.getSize() + "</p>\n");
	int i = 0;
	while(i < resHeaders.getSize()) {
		req.puts("<p>HEADERS: " + resHeaders[i].getKey() + "=>" + resHeaders[i].getVal() + "</p>\n");
		i = i + 1;
	}
	return OK;
}
