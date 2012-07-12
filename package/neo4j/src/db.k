/*
 * written by motoki yoan
 */

/* ----------------------------------------------------------------- */

class GraphDatabaseService {
	String url;
	int port;
	String cypher;
	String rel_idx;
	String node_url;
	String rel_type;
	String version;
	String batch;
	String ex_info;
	String node_idx;
	String ref_node
	Curl c;
	Json json;

	// Constructor
	GraphDatabaseService(String url, int port) {
		this.url = url;
		this.port = port;
		String sport = "" + this.port
		String str = _get(this.url + ":" + sport + "/db/data/");
		Json parsed = json.parse(str);
		cypher =   parsed.getString("cypher");
		rel_idx =  parsed.getString("relationship_index");
		node_url = parsed.getString("node");
		rel_type = parsed.getString("relationship_types");
		version =  parsed.getString("neo4j_version");
		batch =    parsed.getString("batch");
		ex_info =  parsed.getString("extensions_info");
		node_idx = parsed.getString("node_index");
		ref_node = parsed.getString("reference_node");
		//p(cypher); p(rel_idx); p(node_url);
		//p(rel_type); p(version); p(batch);
		//p(ex_info); p(node_idx); p(ref_node);
	}

	void shutdown() {
	}

	Node createNode() {
		String str = _post(this.node_url, "");
		//p(str);
		Node node = new Node(this.node_url, str);
		return node;
	}

	Node getNode(int idx) {
		String sidx = "" + idx;
		String str = _get(this.node_url + "/" + sidx);
		Node node = new Node(this.node_url, str);
		return node;
	}

	Relationship getRelationship(int idx) {
		String sport = "" + this.port
		String sidx = "" + idx;
		String str = _get(this.url + ":" + sport + "/db/data/relationship/" + sidx);
		Relationship rel = new Relationship(this.url + ":" + sport + "/db/data/relationships", str);
		return rel;
	}
}

