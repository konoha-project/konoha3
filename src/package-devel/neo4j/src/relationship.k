/*
 * written by motoki yoan
 */

/* ----------------------------------------------------------------- */

class Relationship {
	int idx;
	Json json;
	String start;
	Json data;
	String end;
	String self;
	String property;
	String properties;
	String type;
	//Node start_node;
	//Node end_Node;

	//Relationship (String url, String query) {
	Relationship (String url, String str) {
		//Json p = json.parse(_post(url, query));
		//p(url);
		Json p = json.parse(str);
		start = p.getString("start");
		end = p.getString("end");
		self = p.getString("self");
		property = p.getString("property");
		properties = p.getString("properties");
		type = p.getString("type");
		data = p.get("data");
		//p(self);
		String relidx = this.self.substring(url.getSize(), this.self.getSize() - url.getSize() + 1);
		this.idx = relidx;
		//p(idx);
		//p(start); p(end); p(self);
		//p(property); p(properties); p(type);
	}
}

