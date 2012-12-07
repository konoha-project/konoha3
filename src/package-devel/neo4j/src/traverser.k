/*
 * written by motoki yoan
 */

/* ----------------------------------------------------------------- */

import("konoha.pcre");

class Traverser {
	String order;
	Json   ret_filter;
	String prune_Eval;
	String uniqueness;
	Json[] rels;
	int    maxdepth;
	Json body;
	Json json;

	// Constructor
	Traverser() {
		body = new Json();
	}

	//Node[] traverse(Node root){
	Json[] traverse(Node root) {
		String t_url = root.traverse.replace(/{.+}/, "node");
		String str = _post(t_url, body.dump());
		Json p = json.parse(str);
		return p.getArray("");
	}

	void setOrder(String order) {
		this.order = order;
		body.setString("order", order);
	}

	void setRelationship(String direction, String type) {
		Json[] ja = new Json[0];
		Json elem = new Json();
		elem.setString("direction", direction);
		elem.setString("type", type);
		ja.add(elem);
		body.setArray("relationships", ja);
	}

	void addRelationship(String direction, String type) {
		Json[] ja = body.getArray("relationships");
		Json elem = new Json();
		elem.setString("direction", direction);
		elem.setString("type", type);
		ja.add(elem);
		body.setArray("relationships", ja);
		//p(rel.dump());
	}

	void setMaxDepth(int depth) {
		this.maxdepth = depth;
		body.setInt("max_depth", depth);
	}
}

