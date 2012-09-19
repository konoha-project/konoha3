load("neo4j.k");

void main() {
	GraphDatabaseService db = new GraphDatabaseService("http://localhost", 7474);
	Traverser t = new Traverser();
	t.setOrder("depth_first");
	t.setRelationship("all", "referTo");
	t.setMaxDepth(3);
	Node root = db.getNode(798);
	Json[] result = t.traverse(root);
	int i = 0;
	while (i < result.getSize()) {
		Json data = result[i].get("data");
		System.p(data.getString("title"));
		System.p(data.getString("sentence"));
		i += 1;
	}
}
main();
