/*
 * written by motoki yoan
 */

load("neo4j.k");

Node addNode(GraphDatabaseService db, Node from, int i) {
	if(i > 2) return null;
	Node node = db.createNode();
	node.setPropertyString("title", "Goal");
	node.setPropertyString("sentence", "hoge");

	//p(node.idx);
	Relationship rel = node.relationshipFrom(from, "referTo");
	addNode(db, node, i+1);
	addNode(db, node, i+1);
	addNode(db, node, i+1);
	return node
}

void goNext() 
{
}
void main() {
	GraphDatabaseService db = new GraphDatabaseService("http://localhost", 7474);
	Node n1 = db.createNode();
	p(n1.idx);
	addNode(db, n1, 0);

	//Node n3 = db.getNode(n1.idx);
	//p(n1.getPropertyString("name"));
	//p(db.getRelationship(rel.idx).type);
	//p(n2.getPropertyString("name"));
	db.shutdown();
}

main();
