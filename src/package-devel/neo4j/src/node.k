/*
 * written by motoki yoan
 */

class Node {
	String outgoing_rel;
	Json data;
	String traverse;
	String all_typed_rel;
	String self;
	String properties;
	String outgoing_typed_rel;
	String incoming_rel;
	String create_rel;
	String paged_traverse;
	String all_rel;
	String incoming_typed_rel;
	int idx;
	String url;
	Json json;

	//Node (String url, String query) {
	Node (String url, String str) {
		this.url = url;
		//String str = "";
		//str = _post(url, query);
		//p(str);
		Json p = json.parse(str);
		this.outgoing_rel = p.getString("outgoing_relationship");
		this.data = p.get("data");
		this.traverse = p.getString("traverse");
		this.all_typed_rel = p.getString("all_typed_relationship");
		this.self = p.getString("self");
		this.properties = p.getString("properties");
		this.outgoing_typed_rel = p.getString("outgoing_typed_relationship");
		this.incoming_rel = p.getString("incoming_relationship");
		this.create_rel = p.getString("create_relationship");
		this.paged_traverse = p.getString("paged_traverse");
		this.all_rel = p.getString("all_relationships");
		this.incoming_typed_rel = p.getString("incoming_typed_relationhip");
		String nodeidx = this.self.substring(url.getSize() + 1, this.self.getSize() - url.getSize() + 1);
		this.idx = nodeidx;
	}

	Relationship relationshipTo(Node n, String type) {
		Json query = new Json();
		query.setString("to", n.self);
		query.setString("type", type);
		String str = _post(this.self + "/relationships", query.dump());
		//Relationship rel = new Relationship(this.self + "/relationships", str);
		Relationship rel = new Relationship("http://localhost:7474/db/data/relationships", str);
		return rel;
	}

	Relationship relationshipFrom(Node n, String type) {
		Json query = new Json();
		query.setString("to", this.self);
		query.setString("type", type);
		//p(query.dump());
		String str = _post(n.self + "/relationships", query.dump());
		//Relationship rel = new Relationship(this.self + "/relationships", str);
		Relationship rel = new Relationship("http://localhost:7474/db/data/relationships", str);
		return rel;
	}

	void setPropertyString(String key, String value) {
		//p(this.properties + "/" + key);
		String str = _put(this.properties + "/" + key, value);
		//p(str);
	}

	void setPropertyInt(String key, int value) {
		String stringValueue = "" + value
		Stirng str = _post(this.properties + "/" + key, stringValueue);
	}

	void setPropertyFloat(String key, Float value) {
		String stringValueue = "" + value
		String str = _post(this.properties + "/" + key, stringValueue);
	}

	void setPropertyBool(String key, Boolean value) {
		String str;
		if(value) {
			str = _post(this.properties + "/" + key, "true"); // [FIXME] is it ok?
		}
		else {
			str = _post(this.properties + "/" + key, "false");
		}
	}

	String getPropertyString(String key) {
		Json p = json.parse(_get(this.properties));
		return p.getString(key);
	}

	int getPropertyInt(String key) {
		Json p = json.parse(_get(this.properties));
		return p.getInt(key);
	}

	Float getPropertyFloat(String key) {
		Json p = json.parse(_get(this.properties));
		return p.getFloat(key);
	}

	Boolean getPropertyBool(String key) {
		Json p = json.parse(_get(this.properties));
		return p.getBool(key);
	}

	Json[] getRelationships() {
		String str = _get(this.all_rel);
		Json p = json.parse(str);
		//Json p = json.parse(_get(this.all_rel));
		Json[] a = p.getArray("");
		return a;
	}

	Node getStartNode(String type) {
		Json[] p = this.getRelationships();
		int size = p.getSize();
		int i = 0;
		while (i < size) {
			if(type == p[i].getString("type")) {
				break;
			}
			i += 1;
		}
		String start_url = p[i].getString("start");
		String str = _get(start_url);
		//p(str);
		return new Node(this.url, str);
	}

	Node getEndNode(String type) {
		Json[] p = this.getRelationships();
		int size = p.getSize();
		int i = 0;
		while (i < size) {
			if(type == p[i].getString("type")) {
				break;
			}
			i += 1;
		}
		String end_url = p[i].getString("end");
		String str = _get(end_url);
		//p(str);
		return new Node(this.url, str);
	}

}

