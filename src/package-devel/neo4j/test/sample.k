/*
 * written by motoki yoan
 */

load("neo4j.k");

void main() {
	// create DB
	GraphDatabaseService db = new GraphDatabaseService("http://localhost", 7474);

	// create Node1: Naruto
	Node n1 = db.createNode();
	n1.setPropertyString("name", "Naruto");
	System.p(n1.getPropertyString("name"));

	// create Node2: Sakura
	Node n2 = db.createNode();
	n2.setPropertyString("name", "Sakura");
	System.p(n2.getPropertyString("name"));

	// create Node2: Sakura
	Node n3 = db.createNode();
	n3.setPropertyString("name", "Sasuke");
	System.p(n3.getPropertyString("name"));

	// create Relationship: Naruto knows Sakura
	Relationship rel = n1.relationshipTo(n2, "LOVES");

	// create Relationship: Naruto knows Sakura
	Relationship rel = n1.relationshipTo(n3, "KNOWS");

	System.p(" ------------------ ");
	// get Node from index
	Node n4 = db.getNode(n1.idx); // n4 is Naruto.
	System.p(n4.getPropertyString("name"));

	// get Naruto's Relationship: "Knows"
	Json[] types = n1.getRelationships();
	//System.p(types[0].getString("type"));

	// get end node KNOWS: Naruto -[KNOWS]-> Sakura
	Node n5 = n1.getEndNode("KNOWS");
	System.p(n5.getPropertyString("name"));

	// get end node KNOWS: Naruto -[KNOWS]-> Sakura
	Node n6 = n1.getEndNode("LOVES");
	System.p(n6.getPropertyString("name"));

	db.shutdown();
}

main();
